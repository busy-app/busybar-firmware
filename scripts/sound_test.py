import os
from flipper.app import App
import subprocess

class Main(App):
    def init(self):
        self.parser.add_argument("source_path", help="Source path")
        self.parser.add_argument("result_path", help="Result path")
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

    def main(self):
        args = self.parser.parse_args()
        
        if not os.path.isdir(args.source_path):
            print(f"Error: {args.source_path} doesn't exist")
        elif not os.path.isdir(args.result_path):
            print(f"Error: {args.result_path} doesn't exist")            
        else:
            
            folders = ["START", "BACK", "OK"]

            result_base_path = f"{args.result_path}\sound"
            os.mkdir(result_base_path)
            name_file = open(result_base_path + '\\' + "names.txt", "w")

            for folder in folders:
                w = os.walk(args.source_path +'\\'+folder)
                for (dirpath, dirnames, filenames) in w:

                    if folder in dirpath:                                                                      
                        result_folder_path = result_base_path +'\\'+folder
                        os.mkdir(result_folder_path)                      

                        wav_files = [f for f in filenames if f.endswith(".wav")]
                        
                        if len(wav_files) > 0:
                            name_file.write(f"{folder}:\r\n")

                        i = 0
                        for file in wav_files:      
                            new_file =  f"{folder.lower()}_{str(i)}.snd"
                            source = dirpath + "\\" + file    
                            dest = result_folder_path + "\\"+ new_file
                            self.convert(source, dest)
                            i += 1                       
                            name_file.write(f"\t{new_file} -------> {file}\r\n")

            name_file.close()                            
        return 0

if __name__ == "__main__":
    Main()()

