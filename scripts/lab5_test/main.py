import sys
from client import Client
from interpreter import Interpreter

class TigerInterpreter(object):

    def main():
        if len(sys.argv) < 2:
            print("Usage:main.py <filename>")
        elif sys.argv[1] == '-d':
            file = sys.argv[2]
            client = Client(file)
            client.cmdloop()
        else:
            file = sys.argv[1]
            interpreter = Interpreter(file)
            interpreter.run()


    if __name__ == "__main__":
        main()