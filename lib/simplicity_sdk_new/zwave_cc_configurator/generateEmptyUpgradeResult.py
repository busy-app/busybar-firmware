import argparse
import os
if __name__ == '__main__':  
  parser = argparse.ArgumentParser(description='This is an empty upgrade result generator for the Z-Wave CC Configurator.')
  parser.add_argument('-r', '--results',
                      help='A path of YAML file that should be written to with upgrade results.')
  args = parser.parse_args()  
  if args.results is not None:
    resultFile = open(args.results, "w")  
    lines = ["---\n","upgrade_results:\n",'-\n','  message: "no upgrade needed"\n', '  status: "nothing"\n']
    resultFile.writelines(lines)
    resultFile.close()
