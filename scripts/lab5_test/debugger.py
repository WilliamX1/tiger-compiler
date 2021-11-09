from interpreter import Interpreter

class Debugger(Interpreter):
    """Interpreter is used to parse executing lines.

    Debug options provide:
        ni
        continue
        breakpoint
        disable breakpoint
        show breakpoints
        disassemble
        show registers
        show temper registers
        show machine registers
        show memory (print by table or by order)
        Error handler

    Attributes:
        
    """
    
    
    def __init__(self, filename):
        super(Debugger, self).__init__(filename)
        self._breakpoints = []
        self._breakpoints_name = {}
        self._pc = self._state_table.get_pc()
            

    # def _print(self, string):
    #     print("(debugger) "+ string)

    # def _print_lines(self, strings):
    #     length = len(strings)
    #     if length > 0:
    #         print("(debugger) " + strings[0])
    #         for i in range(1, length):
    #             print("           " + strings[i])

    def _print_programline(self, pc):
        start = '    ' if self._pc != pc else '===>'
        print('{}{:d}\t\t{}'.format(start, pc, self._program_lines[pc]))

    def next_instruction(self):
        self._print_programline(self._pc)
        if self._pc >= 0:
            self._instructions[self._pc].execute(self._state_table)
            self._pc = self._state_table.get_pc()

        else:
            print("Program Finished!")

    def con(self):
        if self._pc < 0:
            print("Program Finished!")
            return

        stop_exec = False
        self._instructions[self._pc].execute(self._state_table)
        self._pc = self._state_table.get_pc()
        while self._pc >= 0:
            for b in range(len(self._breakpoints)):
                if self._breakpoints[b] == self._pc:
                    breakpoint_index = self._breakpoints[b]
                    if b in self._breakpoints_name:
                        print("Breakpoints %d, %s" % (b+1, self._breakpoints_name[b]))
                    else:
                        print("Breakpoints %d, line %d" % (b+1, breakpoint_index))
                    stop_exec = True
                    break
            if stop_exec:
                break
            self._instructions[self._pc].execute(self._state_table)
            self._pc = self._state_table.get_pc()
            
    def breakpoints_func(self, name):
        if name not in self._label_table:
            print("Function " + name + " not found")
        else:
            index = len(self._breakpoints)
            self._breakpoints.append(self._label_table[name])
            self._breakpoints_name[index] = name
            print("Breakpoints %d for %s in line %d" % (index+1, name, self._label_table[name]))

    def breakpoints_line(self, line):
        if line > len(self._instructions):
            print("Program only has %d lines" % (len(self._instructions)))
        else:
            self._breakpoints.append(line)
            index = len(self._breakpoints)
            print("Breakpoints %d for line %d" % (index, line))

    def disable_all(self):
        for i in range(0, len(self._breakpoints)):
            self._breakpoints[i] = -1
            print("Disable breakpoint %d" % (i+1))

    def disable(self, i):
        if i > len(self._breakpoints) or i < 1:
            print("Unknown breakpoint: %d" % (i))
        else:
            self._breakpoints[i-1] = -1
            print("Disable breakpoint %d" %(i))

    def show_breakpoints(self):
        for i in range(0, len(self._breakpoints)):
            if self._breakpoints[i] == -1:
                ## Disabled breakpoints
                continue
            else:
                breakpoint_index = self._breakpoints[i]
                if i in self._breakpoints_name:
                    print("Breakpoints %d, %s" % (i+1, self._breakpoints_name[i]))
                else:
                    print("Breakpoints %d, line %d" % (i+1, breakpoint_index))

    def disassemble(self):
        #print 20 lines of code
        current_func = self._state_table._current_func
        print(current_func)
        start = self._state_table._label_table[current_func]
        end = self._fun_end[current_func]
        # blank first line
        first_line = self._pc - 10 if self._pc > start + 10 else start
        last_line  = self._pc + 20 if self._pc + 20 < end else end
        for line in range(first_line, first_line + 20):
            if line >= len(self._program_lines):
                break
            self._print_programline(line)


    def _get_temp_regsiters(self):
        state_table = self._state_table
        for temp in state_table._temp_table:
            # print("{} {}{:x}".format(temp, '\t\t0x', state_table._temp_table[temp]))
            print("{} {}{:d}".format(temp, '\t\t', state_table._temp_table[temp]))

    def _get_machine_registers(self):
        state_table = self._state_table
        for reg in state_table._reg_table:
            # print("{} {}{:x}".format(reg, '\t\t0x', state_table._reg_table[reg]))
            print("{} {}{:d}".format(reg, '\t\t', state_table._reg_table[reg]))

    def show_registers(self):
        print("Show both Machine registers and Temp registers")
        print("name\t\tvalue")
        self._get_temp_regsiters()
        self._get_machine_registers()
    
    def show_temp_registers(self):
        print("Show Temp registers")
        print("name\t\tvalue")
        self._get_temp_regsiters()

    def show_machine_registers(self):
        print("Show Machine registers")
        print("name\t\tvalue")
        self._get_machine_registers()

    def show_memory_by_table(self):
        print("Show memory value by table")
        print("  address  \t   value")
        mem_table = self._state_table.get_mem_table()
        for mem in mem_table:
            if isinstance(mem_table[mem], int):
                print("0x%x\t  %d" % (mem, mem_table[mem]))
            else:
                string = mem_table[mem]
                string = string.replace('\\n', '\n')
                string = string.replace('\\t', '\t')
                print('0x%x\t  "%s"' % (mem, string))

    def show_memory_in_order(self):
        print("Show memory value in order")
        print("  address  \t   value")
        mem_table = self._state_table.get_mem_table()
        addresses = list(mem_table.keys())
        addresses.sort()
        for add in addresses:
            if isinstance(mem_table[add], int):
                print("0x%x\t  %d" % (add, mem_table[add]))
            else:
                string = mem_table[add]
                string = string.replace('\n', '\\n')
                string = string.replace('\t', '\\t')
                print('0x%x\t  "%s"' % (add, string))
        
    #     command line ui
    #     Error handler
