/**
 * @file    g10-asm/keyword_table.cpp
 * @author  Dennis W. Griffin <dgdev1024@gmail.com>
 * @date    2025-12-18
 * 
 * @brief   Contains definitions for the G10 CPU assembler's keyword table.
 */

/* Private Includes ***********************************************************/

#include <g10/cpu.hpp>
#include <g10-asm/keyword_table.hpp>

/* Private Static Members *****************************************************/

namespace g10asm
{
    const std::vector<keyword> keyword_table::s_lookup_table =
    {
        // Register Names
        { "D0",     keyword_type::register_name, std::to_underlying(g10::register_type::d0)     },
        { "D1",     keyword_type::register_name, std::to_underlying(g10::register_type::d1)     },
        { "D2",     keyword_type::register_name, std::to_underlying(g10::register_type::d2)     },
        { "D3",     keyword_type::register_name, std::to_underlying(g10::register_type::d3)     },
        { "D4",     keyword_type::register_name, std::to_underlying(g10::register_type::d4)     },
        { "D5",     keyword_type::register_name, std::to_underlying(g10::register_type::d5)     },
        { "D6",     keyword_type::register_name, std::to_underlying(g10::register_type::d6)     },
        { "D7",     keyword_type::register_name, std::to_underlying(g10::register_type::d7)     },
        { "D8",     keyword_type::register_name, std::to_underlying(g10::register_type::d8)     },
        { "D9",     keyword_type::register_name, std::to_underlying(g10::register_type::d9)     },
        { "D10",    keyword_type::register_name, std::to_underlying(g10::register_type::d10)    },
        { "D11",    keyword_type::register_name, std::to_underlying(g10::register_type::d11)    },
        { "D12",    keyword_type::register_name, std::to_underlying(g10::register_type::d12)    },
        { "D13",    keyword_type::register_name, std::to_underlying(g10::register_type::d13)    },
        { "D14",    keyword_type::register_name, std::to_underlying(g10::register_type::d14)    },
        { "D15",    keyword_type::register_name, std::to_underlying(g10::register_type::d15)    },
        { "W0",     keyword_type::register_name, std::to_underlying(g10::register_type::w0)     },
        { "W1",     keyword_type::register_name, std::to_underlying(g10::register_type::w1)     },
        { "W2",     keyword_type::register_name, std::to_underlying(g10::register_type::w2)     },
        { "W3",     keyword_type::register_name, std::to_underlying(g10::register_type::w3)     },
        { "W4",     keyword_type::register_name, std::to_underlying(g10::register_type::w4)     },
        { "W5",     keyword_type::register_name, std::to_underlying(g10::register_type::w5)     },
        { "W6",     keyword_type::register_name, std::to_underlying(g10::register_type::w6)     },
        { "W7",     keyword_type::register_name, std::to_underlying(g10::register_type::w7)     },
        { "W8",     keyword_type::register_name, std::to_underlying(g10::register_type::w8)     },
        { "W9",     keyword_type::register_name, std::to_underlying(g10::register_type::w9)     },
        { "W10",    keyword_type::register_name, std::to_underlying(g10::register_type::w10)    },
        { "W11",    keyword_type::register_name, std::to_underlying(g10::register_type::w11)    },
        { "W12",    keyword_type::register_name, std::to_underlying(g10::register_type::w12)    },
        { "W13",    keyword_type::register_name, std::to_underlying(g10::register_type::w13)    },
        { "W14",    keyword_type::register_name, std::to_underlying(g10::register_type::w14)    },
        { "W15",    keyword_type::register_name, std::to_underlying(g10::register_type::w15)    },
        { "H0",     keyword_type::register_name, std::to_underlying(g10::register_type::h0)     },
        { "H1",     keyword_type::register_name, std::to_underlying(g10::register_type::h1)     },
        { "H2",     keyword_type::register_name, std::to_underlying(g10::register_type::h2)     },
        { "H3",     keyword_type::register_name, std::to_underlying(g10::register_type::h3)     },
        { "H4",     keyword_type::register_name, std::to_underlying(g10::register_type::h4)     },
        { "H5",     keyword_type::register_name, std::to_underlying(g10::register_type::h5)     },
        { "H6",     keyword_type::register_name, std::to_underlying(g10::register_type::h6)     },
        { "H7",     keyword_type::register_name, std::to_underlying(g10::register_type::h7)     },
        { "H8",     keyword_type::register_name, std::to_underlying(g10::register_type::h8)     },
        { "H9",     keyword_type::register_name, std::to_underlying(g10::register_type::h9)     },
        { "H10",    keyword_type::register_name, std::to_underlying(g10::register_type::h10)    },
        { "H11",    keyword_type::register_name, std::to_underlying(g10::register_type::h11)    },
        { "H12",    keyword_type::register_name, std::to_underlying(g10::register_type::h12)    },
        { "H13",    keyword_type::register_name, std::to_underlying(g10::register_type::h13)    },
        { "H14",    keyword_type::register_name, std::to_underlying(g10::register_type::h14)    },
        { "H15",    keyword_type::register_name, std::to_underlying(g10::register_type::h15)    },
        { "L0",     keyword_type::register_name, std::to_underlying(g10::register_type::l0)     },
        { "L1",     keyword_type::register_name, std::to_underlying(g10::register_type::l1)     },
        { "L2",     keyword_type::register_name, std::to_underlying(g10::register_type::l2)     },
        { "L3",     keyword_type::register_name, std::to_underlying(g10::register_type::l3)     },
        { "L4",     keyword_type::register_name, std::to_underlying(g10::register_type::l4)     },
        { "L5",     keyword_type::register_name, std::to_underlying(g10::register_type::l5)     },
        { "L6",     keyword_type::register_name, std::to_underlying(g10::register_type::l6)     },
        { "L7",     keyword_type::register_name, std::to_underlying(g10::register_type::l7)     },
        { "L8",     keyword_type::register_name, std::to_underlying(g10::register_type::l8)     },
        { "L9",     keyword_type::register_name, std::to_underlying(g10::register_type::l9)     },
        { "L10",    keyword_type::register_name, std::to_underlying(g10::register_type::l10)    },
        { "L11",    keyword_type::register_name, std::to_underlying(g10::register_type::l11)    },
        { "L12",    keyword_type::register_name, std::to_underlying(g10::register_type::l12)    },
        { "L13",    keyword_type::register_name, std::to_underlying(g10::register_type::l13)    },
        { "L14",    keyword_type::register_name, std::to_underlying(g10::register_type::l14)    },
        { "L15",    keyword_type::register_name, std::to_underlying(g10::register_type::l15)    },

        // Branching Condition Codes
        { "NC",     keyword_type::condition_code, g10::CC_NO_CONDITION      },
        { "ZS",     keyword_type::condition_code, g10::CC_ZERO_SET          },
        { "ZC",     keyword_type::condition_code, g10::CC_ZERO_CLEAR        },
        { "CS",     keyword_type::condition_code, g10::CC_CARRY_SET         },
        { "CC",     keyword_type::condition_code, g10::CC_CARRY_CLEAR       },
        { "VS",     keyword_type::condition_code, g10::CC_OVERFLOW_SET      },
        { "VC",     keyword_type::condition_code, g10::CC_OVERFLOW_CLEAR    },

        // Directives 
        { ".BYTE",      keyword_type::directive,    std::to_underlying(directive_type::byte)        },
        { ".WORD",      keyword_type::directive,    std::to_underlying(directive_type::word)        },
        { ".DWORD",     keyword_type::directive,    std::to_underlying(directive_type::dword)       },
        { ".ORG",       keyword_type::directive,    std::to_underlying(directive_type::org)         },
        { ".GLOBAL",    keyword_type::directive,    std::to_underlying(directive_type::global_)     },
        { ".EXTERN",    keyword_type::directive,    std::to_underlying(directive_type::extern_)     },

        // Instructions
        { "NOP",        keyword_type::instruction,  std::to_underlying(g10::instruction::nop)       },
        { "STOP",       keyword_type::instruction,  std::to_underlying(g10::instruction::stop)      },
        { "HALT",       keyword_type::instruction,  std::to_underlying(g10::instruction::halt)      },
        { "DI",         keyword_type::instruction,  std::to_underlying(g10::instruction::di)        },
        { "EI",         keyword_type::instruction,  std::to_underlying(g10::instruction::ei)        },
        { "EII",        keyword_type::instruction,  std::to_underlying(g10::instruction::eii)       },
        { "DAA",        keyword_type::instruction,  std::to_underlying(g10::instruction::daa)       },
        { "SCF",        keyword_type::instruction,  std::to_underlying(g10::instruction::scf)       },
        { "CCF",        keyword_type::instruction,  std::to_underlying(g10::instruction::ccf)       },
        { "CLV",        keyword_type::instruction,  std::to_underlying(g10::instruction::clv)       },
        { "SEV",        keyword_type::instruction,  std::to_underlying(g10::instruction::sev)       },
        { "LD",         keyword_type::instruction,  std::to_underlying(g10::instruction::ld)        },
        { "LDQ",        keyword_type::instruction,  std::to_underlying(g10::instruction::ldq)       },
        { "LDP",        keyword_type::instruction,  std::to_underlying(g10::instruction::ldp)       },
        { "ST",         keyword_type::instruction,  std::to_underlying(g10::instruction::st)        },
        { "STQ",        keyword_type::instruction,  std::to_underlying(g10::instruction::stq)       },
        { "STP",        keyword_type::instruction,  std::to_underlying(g10::instruction::stp)       },
        { "MV",         keyword_type::instruction,  std::to_underlying(g10::instruction::mv)        },
        { "MWH",        keyword_type::instruction,  std::to_underlying(g10::instruction::mwh)       },
        { "MWL",        keyword_type::instruction,  std::to_underlying(g10::instruction::mwl)       },
        { "LSP",        keyword_type::instruction,  std::to_underlying(g10::instruction::lsp)       },
        { "POP",        keyword_type::instruction,  std::to_underlying(g10::instruction::pop)       },
        { "SSP",        keyword_type::instruction,  std::to_underlying(g10::instruction::ssp)       },
        { "PUSH",       keyword_type::instruction,  std::to_underlying(g10::instruction::push)      },
        { "SPO",        keyword_type::instruction,  std::to_underlying(g10::instruction::spo)       },
        { "SPI",        keyword_type::instruction,  std::to_underlying(g10::instruction::spi)       },
        { "JMP",        keyword_type::instruction,  std::to_underlying(g10::instruction::jmp)       },
        { "JPB",        keyword_type::instruction,  std::to_underlying(g10::instruction::jpb)       },
        { "CALL",       keyword_type::instruction,  std::to_underlying(g10::instruction::call)      },
        { "INT",        keyword_type::instruction,  std::to_underlying(g10::instruction::int_)      },
        { "RET",        keyword_type::instruction,  std::to_underlying(g10::instruction::ret)       },
        { "RETI",       keyword_type::instruction,  std::to_underlying(g10::instruction::reti)      },
        { "ADD",        keyword_type::instruction,  std::to_underlying(g10::instruction::add)       },
        { "ADC",        keyword_type::instruction,  std::to_underlying(g10::instruction::adc)       },
        { "SUB",        keyword_type::instruction,  std::to_underlying(g10::instruction::sub)       },
        { "SBC",        keyword_type::instruction,  std::to_underlying(g10::instruction::sbc)       },
        { "INC",        keyword_type::instruction,  std::to_underlying(g10::instruction::inc)       },
        { "DEC",        keyword_type::instruction,  std::to_underlying(g10::instruction::dec)       },
        { "AND",        keyword_type::instruction,  std::to_underlying(g10::instruction::and_)      },
        { "OR",         keyword_type::instruction,  std::to_underlying(g10::instruction::or_)       },
        { "XOR",        keyword_type::instruction,  std::to_underlying(g10::instruction::xor_)      },
        { "NOT",        keyword_type::instruction,  std::to_underlying(g10::instruction::not_)      },
        { "CMP",        keyword_type::instruction,  std::to_underlying(g10::instruction::cmp)       },
        { "SLA",        keyword_type::instruction,  std::to_underlying(g10::instruction::sla)       },
        { "SRA",        keyword_type::instruction,  std::to_underlying(g10::instruction::sra)       },
        { "SRL",        keyword_type::instruction,  std::to_underlying(g10::instruction::srl)       },
        { "SWAP",       keyword_type::instruction,  std::to_underlying(g10::instruction::swap)      },
        { "RLA",        keyword_type::instruction,  std::to_underlying(g10::instruction::rla)       },
        { "RL",         keyword_type::instruction,  std::to_underlying(g10::instruction::rl)        },
        { "RLCA",       keyword_type::instruction,  std::to_underlying(g10::instruction::rlca)      },
        { "RLC",        keyword_type::instruction,  std::to_underlying(g10::instruction::rlc)       },
        { "RRA",        keyword_type::instruction,  std::to_underlying(g10::instruction::rra)       },
        { "RR",         keyword_type::instruction,  std::to_underlying(g10::instruction::rr)        },
        { "RRCA",       keyword_type::instruction,  std::to_underlying(g10::instruction::rrca)      },
        { "RRC",        keyword_type::instruction,  std::to_underlying(g10::instruction::rrc)       },
        { "BIT",        keyword_type::instruction,  std::to_underlying(g10::instruction::bit)       },
        { "SET",        keyword_type::instruction,  std::to_underlying(g10::instruction::set)       },
        { "RES",        keyword_type::instruction,  std::to_underlying(g10::instruction::res)       },
        { "TOG",        keyword_type::instruction,  std::to_underlying(g10::instruction::tog)       },
        { "JP",         keyword_type::instruction,  std::to_underlying(g10::instruction::jp)        },
        { "JR",         keyword_type::instruction,  std::to_underlying(g10::instruction::jr)        },
        { "CPL",        keyword_type::instruction,  std::to_underlying(g10::instruction::cpl)       },
        { "CP",         keyword_type::instruction,  std::to_underlying(g10::instruction::cp)        }
        
    };
}

/* Public Methods *************************************************************/

namespace g10asm
{

    auto keyword_table::lookup (std::string_view lexeme) 
        -> g10::result_cref<keyword>
    {
        // - Search the keyword lookup table for a matching lexeme.
        auto it = std::find_if(
            s_lookup_table.begin(),
            s_lookup_table.end(),
            [lexeme] (const keyword& kw) -> bool
            {
                // - Perform a case-insensitive comparison of the lexemes.
                return std::equal(
                    lexeme.begin(),
                    lexeme.end(),
                    kw.lexeme.begin(),
                    kw.lexeme.end(),
                    [] (char a, char b) -> bool
                    {
                        // - `a` is from `lexeme` and needs to be uppercased.
                        // - `b` is from `kw.lexeme` and is already uppercase.
                        return std::toupper(a) == b;
                    }
                );
            }
        );

        // - If a matching keyword was found, return it.
        if (it != s_lookup_table.end())
        {
            return std::cref(*it);
        }
        else
        {
            return error("Lexeme '{}' is not a recognized keyword.",
                lexeme);
        }
    }

}
