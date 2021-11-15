from state_table import StateTable
from type import InstructionEnum
import re
from instruction import *

class Interpreter(object):
    """Interpreter is used to parse executing lines.

    This class will store state table of program, 

    Attributes:
        state_table: A boolean indicating if we like SPAM or no.
        eggs: An integer count of the eggs we have laid.
    """

    def __init__(self, filename):
        self._label_table = {}
        self._string_table = {}
        self._program_lines = []
        self._fun_end = {}
        self._frame_size = {}
        self._scan_lines(filename)
        self._state_table = StateTable(self._label_table, self._string_table, self._frame_size)
        self._instructions = []
        self._init_instructions()
        

    def _scan_lines(self, filename):
        with open(filename, 'r') as f:
            cursor = 0
            last_label = ''
            line = f.readline()
            def scan_strings():
                line = f.readline()
                while line:
                    string_label = line[:-2]
                    line = f.readline()
                    line = f.readline()
                    string = line.partition(' ')[2][:-1]
                    string = string.replace('\"', '')
                    string = string.replace('\\n', '\n')
                    string = string.replace('\\t', '\t')
                    self._string_table[string_label] = string
                    line = f.readline()
            while line:
                line = line.strip()
                if len(line) == 0:
                    # Ignore blank line
                    line = f.readline()
                    continue

                elif line.startswith('.section .rodata'):
                    # start string part
                    scan_strings()
                    break

                elif line.startswith('.set'):
                    frame_size = line.split(' ')
                    self._frame_size[frame_size[1][:-1]] = frame_size[2]
                    line = f.readline()
                    continue

                elif line.startswith('.'):
                    # We only store instructions
                    line = f.readline()
                    continue

                elif line == 'cqto':
                    # Ignore cqto
                    line = f.readline()
                    continue

                elif line.endswith(':'):
                    # Store label in line_map
                    label = line[0:-1]
                    self._label_table[label] = cursor
                    if len(last_label) > 0:
                        self._fun_end[last_label] = cursor - 1
                    last_label = label
                    line = f.readline()
                    continue

                else:
                    self._program_lines.append(line)
                    cursor = cursor + 1
                    line = f.readline()

    def _init_instructions(self):
        def _parse_args(line, num):
            parts = line.partition(' ')
            args  = parts[2].split(',')
            assert len(args) == num, 'Wrong args number in instruction'
            args_res = []
            for i in range(num):
                args_res.append(args[i].strip())
            return args_res

        def _parse_line(line):
            if re.match(InstructionEnum.Mov.value, line):
                args = _parse_args(line, 2)
                return MovInstruction(InstructionEnum.Mov, args[0], args[1])
        
            elif re.match(InstructionEnum.Add.value, line):
                args = _parse_args(line, 2)
                return AddInstruction(InstructionEnum.Add, args[0], args[1])
            
            elif re.match(InstructionEnum.Sub.value, line):
                args = _parse_args(line, 2)
                return SubInstruction(InstructionEnum.Sub, args[0], args[1])
            
            elif re.match(InstructionEnum.Imul.value, line):
                args = _parse_args(line, 1)
                return ImulInstruction(InstructionEnum.Imul, args[0])
            
            elif re.match(InstructionEnum.Idiv.value, line):
                args = _parse_args(line, 1)
                return IdivInstruction(InstructionEnum.Idiv, args[0])
            
            elif re.match(InstructionEnum.Lea.value, line):
                args = _parse_args(line, 2)
                return LeaInstruction(InstructionEnum.Lea, args[0], args[1])
            
            elif re.match(InstructionEnum.Call.value, line):
                args = _parse_args(line, 1)
                return CallInstruction(InstructionEnum.Call, args[0])

            elif re.match(InstructionEnum.Cmp.value, line):
                args = _parse_args(line, 2)
                return CmpInstruction(InstructionEnum.Cmp, args[0], args[1])

            elif re.match(InstructionEnum.Jmp.value, line):
                space_index = line.find(' ')
                condition = JumpEnum.Jmp
                for name, member in JumpEnum.__members__.items():
                    if line[:space_index] == member.value:
                        condition = member
                            
                return JmpInstruction(InstructionEnum.Jmp, condition, line[space_index:].strip())

            elif re.match(InstructionEnum.Ret.value, line):
                return RetInstruction(InstructionEnum.Ret)
            
            else:
                raise NotImplementedError(line + " is not a valid instruction!")
        
        for line in self._program_lines:
            self._instructions.append(_parse_line(line))

    def run(self):
        pc = self._state_table.get_pc()
        while pc >= 0:
            # print(pc)
            # print(self._program_lines[pc])
            # print(self._instructions[pc])
            # self._instructions[pc].print_kind()
            self._instructions[pc].execute(self._state_table)
            
            pc = self._state_table.get_pc()

