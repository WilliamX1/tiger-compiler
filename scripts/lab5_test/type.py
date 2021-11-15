from enum import Enum,unique
@unique
class InstructionEnum(Enum):
    Mov  = '^mov.+$'
    Add  = '^add.+$'
    Sub  = '^sub.+$'
    Imul = '^imul.+$'
    Idiv = '^idiv.+$'
    Lea  = '^lea.+$'
    Call = '^call.+$'
    Cmp  = '^cmp.+$'
    Jmp  = '^j[(mp)|(e)|(ne)|(g)|(ge)|(l)|(le)].+$'
    Ret  = '^retq$' 

@unique
class OperandEnum(Enum):
    Reg = '^(\%r.{1,2})|(t\d{3})$'
    Imm = '^\$.+$'

@unique
class JumpEnum(Enum):
    Ge  = 'jge'
    G   = 'jg'
    Le  = 'jle'
    L   = 'jl'
    E   = 'je'
    Ne  = 'jne'
    Jmp = 'jmp'
