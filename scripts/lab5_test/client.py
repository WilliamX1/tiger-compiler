from cmd import Cmd
from os import error
import sys
from debugger import Debugger

class Client(Cmd):
    """help
    这是doc
    """
    prompt = '(tiger) '
    intro = 'Using Tiger Interpreter'

    def __init__(self, filename):
        print(filename)
        # reload(sys)
        # sys.setdefaultencoding('utf-8')
        self._debugger = Debugger(filename)
        Cmd.__init__(self)


    def help_show(self):
        print("help")

    def help_disable(self):
        print("disable <i>")
        print("<i> is the index of breakpoint to disable")

    def help_breakpoints(self):
        print("b <line>/func")
        print("<line> is the line number of your assem you want to break, and func is the function name")

    def do_show(self,arg):
        args = arg.split()
        if len(args) < 1:
            self.help_show()
        else:
            if args[0].startswith('r'):
                self._debugger.show_registers()
            elif args[0].startswith('t'):
                self._debugger.show_temper_registers()
            elif args[0].startswith('ma'):
                self._debugger.show_machine_registers()
            elif args[0].startswith('memtable'):
                self._debugger.show_memory_by_table()
            elif args[0].startswith('me'):
                self._debugger.show_memory_in_order()
            elif args[0].startswith('b'):
                self._debugger.show_breakpoints()
            else:
                self.help_show()

    def do_ni(self, arg):
        print(arg)
        if len(arg) > 0 and arg.isdigit():
            for i in range(int(arg)):
                self._debugger.next_instruction()
        else:
            self._debugger.next_instruction()


    def do_continue(self, arg):
        try:
            self._debugger.con()
        except BaseException:
            print('Error occur at:')
            self._debugger.disassemble()

    def do_b(self, arg):
        args = arg.split()
        if len(args) < 1:
            self.help_breakpoints()
        else:
            if args[0].isdigit():
                self._debugger.breakpoints_line(int(args[0]))
            else:
                self._debugger.breakpoints_func(args[0])

    def do_disable(self, arg):
        if len(arg) < 1:
            self._debugger.disable_all()
        else:
            if arg[0].isdigit():
                self._debugger.disable(int(arg[0]))
            else:
                help_breakpoints()
            
    def do_disassemble(self, arg):
        self._debugger.disassemble()


    def do_exit(self,arg):
        print('Exit Tiger Interpreter')
        return True