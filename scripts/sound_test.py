import os
import subprocess
import sys
from flipper.app import App
from flipper.storage_socket import FlipperStorage, FlipperStorageOperations

class ListStream:
    def __init__(self):
        self.data = []
    def write(self, s):
        self.data.append(s)

class Main(App):
    def init(self):         
        self.parser.add_argument("source_path", help="Source path")
        self.parser.add_argument("result_path", help="Result path")
        self.parser.add_argument("-u", dest="update", help="Update bundle on bsb after convertion is done", action='store_true')        
        self.parser.set_defaults(func=self.main)
        
    def convert(self, source_file_path, dest_file_path):
        ffmpeg = (
            "ffmpeg",
            "-i",
            source_file_path,
            "-acodec",
            "pcm_s16le",
            "-f",
            "s16le",
            "-ac",
            "1",
            "-ar",
            "44100",
            dest_file_path,
        )
        print("Running command:", " ".join(ffmpeg))
        subprocess.run(ffmpeg, check=True)

    def prepare_bundle(self, source_path, result_path):    
        if not os.path.isdir(source_path):
            print(f"Error: {source_path} doesn't exist")
            return False
        elif not os.path.isdir(result_path):
            print(f"Error: {result_path} doesn't exist")            
            return False
        else:
            
            folders = ["START", "BACK", "OK"]

            result_base_path = os.path.join(result_path, 'sound')
            os.mkdir(result_base_path)
            name_file = open(os.path.join(result_base_path, "names.txt"), "w")

            for folder in folders:
                w = os.walk(os.path.join(source_path, folder))
                for (dirpath, dirnames, filenames) in w:

                    if folder in dirpath:                                                                      
                        result_folder_path = os.path.join(result_base_path, folder)                        
                        os.mkdir(result_folder_path)

                        wav_files = [f for f in filenames if f.endswith(".wav") or f.endswith(".m4a")]                        
                        if len(wav_files) > 0:
                            name_file.write(f"{folder}:\r\n")

                        i = 0
                        for file in wav_files:      
                            new_file =  f"{folder.lower()}_{str(i)}.snd"
                            source = os.path.join(dirpath, file)
                            dest = os.path.join(result_folder_path, new_file)
                            self.convert(source, dest)
                            i += 1                       
                            name_file.write(f"\t{new_file} -------> {file}\r\n")

            name_file.close()
            return True                      

    def _get_port(self):        
        return ("10.0.4.20", 23)

    def remove_bundle_from_bsb(self): 
        stream = ListStream()
        with FlipperStorage(self._get_port()) as storage:
            sys.stdout = stream
            storage.list_tree("/ext/sound")
            
        sys.stdout = sys.__stdout__
        files = [f for f in stream.data if "." in f]
        for file_path in files:
            with FlipperStorage(self._get_port()) as storage:
                fp = file_path.split(",")[0]
                print(f"Removing {fp}")  
                storage.remove(fp)
        print("All sound removed from BSB")

    def format_to_ext_bsb_path(self, dest_path, filename):
        return "/ext" + os.path.join(dest_path, filename).replace(os.sep,"/")

    def send_bundle_to_bsb(self,result_path):
        w = os.walk(result_path + "\sound")
        for (dirpath, dirnames, filenames) in w:
            snd_files = [f for f in filenames if f.endswith(".snd")]         
            for snd in snd_files:
                with FlipperStorage(self._get_port()) as storage:                                    
                    dest = dirpath.replace(result_path,'')            
                    to_path = self.format_to_ext_bsb_path(dest, snd)

                    from_path = os.path.join(dirpath, snd)
                    print(f"Sending \"{from_path}\" to \"{to_path}\"")
                    storage.send_file(f"{from_path}", f"{to_path}")
        print("Bundle updated!")                               

    def update_bundle(self, result_path):
        self.remove_bundle_from_bsb()
        self.send_bundle_to_bsb(result_path)

    def main(self):
        args = self.parser.parse_args()
        self.prepare_bundle(args.source_path, args.result_path)
        if args.update:
            self.update_bundle(args.result_path)
        return 0
    
if __name__ == "__main__":
    Main()()

