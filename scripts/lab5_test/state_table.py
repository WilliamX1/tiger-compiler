import re
from type import OperandEnum,JumpEnum
import sys

class StateTable(object):
    def __init__(self, label_table, string_table, frame_size):
        self._reg_table = {}
        self._temp_table = {}
        self._mem_table = {}
        self._label_table = label_table
        self._frame_size = frame_size
        self._pc = label_table['tigermain']
        self._copmarison = [0,0]
        self._func_temp_stack = []
        self._func_name_stack = []
        # Init rip and rsp for pc and stack pointer.
        self._string_address = 0x400000
        self._init_reg()
        self._string_index = 0
        self._construct_string(string_table)
        self._heap_address = 0x100000
        self._mem_table[0x200000] = -1
        self._current_func = 'tigermain'

    def _init_reg(self):
        # Init machine registers in x86
        self._reg_table['%rax'] = 0
        self._reg_table['%rbx'] = 0
        self._reg_table['%rcx'] = 0
        self._reg_table['%rdx'] = 0
        self._reg_table['%rsi'] = 0
        self._reg_table['%rdi'] = 0
        self._reg_table['%rbp'] = 0
        self._reg_table['%rsp'] = 0x200000
        self._reg_table['%r8']  = 0
        self._reg_table['%r9']  = 0
        self._reg_table['%r10'] = 0
        self._reg_table['%r11'] = 0
        self._reg_table['%r12'] = 0
        self._reg_table['%r13'] = 0
        self._reg_table['%r14'] = 0
        self._reg_table['%r15'] = 0
        self._reg_table['%rip'] = 0

    def _construct_string(self, string_table):
        for label in string_table.keys():
            index = int(label[1:])
            self._string_index = max(index, self._string_index)
            string_address = self._string_address + index * 8
            self._mem_table[string_address] = string_table[label]

        
    
    def store_reg(self, reg, val):
        if reg.startswith('%'):
            # Machine Registers
            self._reg_table[reg] = val
        elif reg.startswith('t'):
            # Temp Registers
            self._temp_table[reg] = val
        else:
            raise AssertionError(reg + ' is not a valid register')

    def load_reg(self, reg):
        if reg.startswith('%'):
            # Machine Registers
            return self._reg_table[reg]
        elif reg.startswith('t'):
            # Temp Registers
            return self._temp_table[reg]
        else:
            raise AssertionError(reg + ' is not a valid register')

    def store_mem(self, mem_address, val):
        self._mem_table[mem_address] = val
    
    def load_mem(self, mem_address):
        if mem_address not in self._mem_table:
            return 0xbaadbeef
            # raise AssertionError('%lx is not a valid mem address' % (mem_address))
        return self._mem_table.get(mem_address, 0)

    def get_operand(self, operand):
        operand_val = 0
        if re.match(OperandEnum.Reg.value, operand):
            operand_val = self.load_reg(operand)
        elif re.match(OperandEnum.Imm.value, operand):
            if operand.startswith('$0x'):
                operand_val = int(operand[1:], 16)
            else:
                operand_val = int(operand[1:])
        else:
            # print(operand)
            operand_val = self.load_mem(self.get_mem_address(operand))
            # print(self._temp_table)

        return operand_val

    def get_pc(self):
        self._pc = self._pc + 1
        return self._pc - 1
    
    def jump_label(self, label):
        if label in self._label_table:
            self._pc = self._label_table[label]
        else:
            raise ValueError(label + " is not a valid label")
        
    def set_comparison(self, arg0_val, arg1_val):
        self._copmarison = [arg0_val, arg1_val]

    def validate_condition(self, condition):
        arg0_val = self._copmarison[0]
        arg1_val = self._copmarison[1]
        if condition == JumpEnum.Jmp:
            return True
        elif condition == JumpEnum.Ge:
            return arg1_val >= arg0_val
        elif condition == JumpEnum.G:
            return arg1_val > arg0_val
        elif condition == JumpEnum.Le:
            return arg1_val <= arg0_val
        elif condition == JumpEnum.L:
            return arg1_val < arg0_val
        elif condition == JumpEnum.E:
            return arg1_val == arg0_val
        elif condition == JumpEnum.Ne:
            return arg1_val != arg0_val
        else:
            AssertionError("Not a valid jump instruction")

    def call(self, label):
        # print(self._reg_table)
        label = label.replace('@PLT', '')
        if label in self._label_table:
            self._func_name_stack.append(self._current_func)
            self._current_func = label
            rsp = self.load_reg('%rsp') - 8
            self.store_mem(rsp, self._pc)
            self.store_reg('%rsp', rsp)
            self._pc = self._label_table[label]
            self._func_temp_stack.append(self._temp_table.copy())
            self._temp_table.clear()

        elif label == 'print':
            # print(self.load_reg('%rdi'))
            # print(self._string_table)
            string_address = self.load_reg('%rdi')
            print(self._mem_table[string_address], end='')
        
        elif label == 'printi':
            # print(self.load_reg('%rdi'))
            value = self.load_reg('%rdi')
            print(value, end='')
        
        elif label == 'init_array':
            size = self.load_reg('%rdi')
            init = self.load_reg('%rsi')
            array = self._heap_address
            self._heap_address = self._heap_address + size * 8
            # Return heap address
            self.store_reg('%rax', array)
            # Init array
            for i in range(size):
                self.store_mem(array + i * 8, init)
        
        elif label == 'alloc_record':
            size = self.load_reg('%rdi')
            record = self._heap_address
            self.store_reg('%rax', record)
            for i in range(0, size, 8):
                self.store_mem(i * 8 + record, 0)
            self._heap_address = self._heap_address + size
        
        elif label == 'ord':
            string_add = self.load_reg('%rdi')
            if string_add not in self._mem_table:
                self.store_reg('%rax', -1)
            else:
                string = self._mem_table[string_add]
                if len(string) == 0:
                    self.store_reg('%rax', -1)
                else:
                    self.store_reg('%rax', ord(string[0]))
        
        elif label == 'getchar':
            string = sys.stdin.read(1)
            self._string_index = self._string_index + 1
            string_add = self._string_index * 8 + self._string_address
            self.store_mem(string_add, string)
            self.store_reg('%rax', string_add)
        
        elif label == 'chr':
            i = self.load_reg('%rdi')
            ch = ''
            if i < 0 or i >= 256:
                raise ValueError("chr(%d) out of range" % (i))
            else:
                ch = ch + chr(i)

            self._string_index = self._string_index + 1
            ch_add = self._string_index * 8 + self._string_address
            self.store_mem(ch_add, ch)
            self.store_reg('%rax', ch_add)

        elif label == 'string_equal':
            arg0 = self.load_reg('%rdi')
            arg1 = self.load_reg('%rsi')
            ret = 0 if self.load_mem(arg0) != self.load_mem(arg1) else 1
            self.store_reg('%rax', ret)
        else:
            raise ValueError(label + " is not a valid label")

    def ret(self):
        rsp = self.load_reg('%rsp')
        ret_address = self.load_mem(rsp)
        self.store_reg('%rsp', rsp+8)
        self._pc = ret_address
        if len(self._func_temp_stack) > 0:
            self._temp_table = self._func_temp_stack.pop()
            self._current_func = self._func_name_stack.pop()

    def get_mem_address(self, mem):
        def get_number(num):
            return int(num) if num.find('0x') == -1 else int(num, 16)
        mem = mem.strip()
        imm_index = mem.find('(%')
        imm_index = mem.find('(t') if imm_index == -1 else imm_index
        imm = 0
        if imm_index != -1:
            if imm_index == 0:
                imm = 0
            elif mem.find('framesize') != -1:
                imm_part = mem[:imm_index]
                for frame_name in self._frame_size.keys():
                    if imm_part.find(frame_name) != -1:
                        imm_part = imm_part.replace(frame_name, self._frame_size[frame_name])
                        break
                imm_part = imm_part.strip('(')
                imm_part = imm_part.strip(')')
                if imm_part.find('+') != -1:
                    imm_numbers = imm_part.split('+')
                    imm = get_number(imm_numbers[0]) + get_number(imm_numbers[1])
                elif imm_part.find('-') != -1:
                    imm_numbers = imm_part.split('-')
                    imm = get_number(imm_numbers[0]) - get_number(imm_numbers[1])
                else:
                    imm = get_number(imm_part)

            else:
                if mem.startswith('L'):
                    # Relative mem_address
                    return self._string_address + int(mem[1:imm_index]) * 8
                else:
                    imm = int(mem[:imm_index], 16) if mem.startswith('0x') else int(mem[:imm_index])
            mem = mem[imm_index+1:-1]
            regs = mem.split(',')
            # Imm(reg_b, reg_i, s) -> Imm + R[reg_b] + R[reg_i] * s 
            if len(regs) == 3:
                reg_b = 0
                reg_i = 0
                s = 0
                if len(regs[0]) == 0:
                    reg_b = 0
                else:
                    reg_b = self.load_reg(regs[0])
                
                reg_i = self.load_reg(regs[1])
                s = int(regs[2], 16) if regs[2].startswith('0x') else int(regs[2])
                return imm + reg_b + reg_i * s
            # Imm(reg_b, reg_i) -> Imm + R[reg_b] + R[reg_i]
            elif len(regs) == 2:
                reg_b = self.load_reg(regs[0])
                reg_i = self.load_reg(regs[1])
                return imm + reg_b + reg_i
            elif len(regs) == 1:
                reg_a = self.load_reg(regs[0])
                return reg_a + imm
            else:
                return imm
                
        else:
            return int(mem[:imm_index], 16) if mem.startswith('0x') else int(mem[:imm_index])

    def get_mem_table(self):
        return self._mem_table


    


    
