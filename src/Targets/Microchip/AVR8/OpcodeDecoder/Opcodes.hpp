#pragma once

#include <array>

#include "Opcode.hpp"
#include "Instruction.hpp"
#include "src/Services/BitsetService.hpp"

namespace Targets::Microchip::Avr8::OpcodeDecoder::Opcodes
{
    using Adc = Opcode<
        "ADC",
        0b0001110000000000,
        1,
        Instruction::Mnemonic::ADC,
        false,
        RegisterParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {9, 1},
            {3, 4},
        })},
        RegisterParameter{{8, 5}}
    >;

    using Add = Opcode<
        "ADD",
        0b0000110000000000,
        1,
        Instruction::Mnemonic::ADD,
        false,
        RegisterParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {9, 1},
            {3, 4},
        })},
        RegisterParameter{{8, 5}}
    >;

    using Adiw = Opcode<
        "ADIW",
        0b1001011000000000,
        1,
        Instruction::Mnemonic::ADIW,
        false,
        std::nullopt,
        RegisterParameter{24, true, {5, 2}},
        InstructionParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {7, 2},
            {3, 4},
        })}
    >;

    using And = Opcode<
        "AND",
        0b0010000000000000,
        1,
        Instruction::Mnemonic::AND,
        false,
        RegisterParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {9, 1},
            {3, 4},
        })},
        RegisterParameter{{8, 5}}
    >;

    using Andi = Opcode<
        "ANDI",
        0b0111000000000000,
        1,
        Instruction::Mnemonic::ANDI,
        false,
        std::nullopt,
        RegisterParameter{{7, 4}},
        InstructionParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {11, 4},
            {3, 4},
        })}
    >;

    using Asr = Opcode<
        "ASR",
        0b1001010000000101,
        1,
        Instruction::Mnemonic::ASR,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using Bclr = Opcode<
        "BCLR",
        0b1001010010001000,
        1,
        Instruction::Mnemonic::BCLR,
        false,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{6, 3}}
    >;

    using Bld = Opcode<
        "BLD",
        0b1111100000000000,
        1,
        Instruction::Mnemonic::BLD,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}},
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{2, 3}}
    >;

    using Brbc = Opcode<
        "BRBC",
        0b1111010000000000,
        1,
        Instruction::Mnemonic::BRBC,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{9, 7}},
        std::nullopt,
        InstructionParameter{{2, 3}}
    >;

    using Brbs = Opcode<
        "BRBS",
        0b1111000000000000,
        1,
        Instruction::Mnemonic::BRBS,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{9, 7}},
        std::nullopt,
        InstructionParameter{{2, 3}}
    >;

    using Brcc = Opcode<
        "BRCC",
        0b1111010000000000,
        1,
        Instruction::Mnemonic::BRCC,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{9, 7}}
    >;

    using Brcs = Opcode<
        "BRCS",
        0b1111000000000000,
        1,
        Instruction::Mnemonic::BRCS,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{9, 7}}
    >;

    using Break = Opcode<
        "BREAK",
        0b1001010110011000,
        1,
        Instruction::Mnemonic::BREAK,
        false
    >;

    using Breq = Opcode<
        "BREQ",
        0b1111000000000001,
        1,
        Instruction::Mnemonic::BREQ,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{9, 7}}
    >;

    using Brge = Opcode<
        "BRGE",
        0b1111010000000100,
        1,
        Instruction::Mnemonic::BRGE,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{9, 7}}
    >;

    using Brhc = Opcode<
        "BRHC",
        0b1111010000000101,
        1,
        Instruction::Mnemonic::BRHC,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{9, 7}}
    >;

    using Brhs = Opcode<
        "BRHS",
        0b1111000000000101,
        1,
        Instruction::Mnemonic::BRHS,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{9, 7}}
    >;

    using Brid = Opcode<
        "BRID",
        0b1111010000000111,
        1,
        Instruction::Mnemonic::BRID,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{9, 7}}
    >;

    using Brie = Opcode<
        "BRIE",
        0b1111000000000111,
        1,
        Instruction::Mnemonic::BRIE,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{9, 7}}
    >;

    using Brlo = Opcode<
        "BRLO",
        0b1111000000000000,
        1,
        Instruction::Mnemonic::BRLO,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{9, 7}}
    >;

    using Brlt = Opcode<
        "BRLT",
        0b1111000000000100,
        1,
        Instruction::Mnemonic::BRLT,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{9, 7}}
    >;

    using Brmi = Opcode<
        "BRMI",
        0b1111000000000010,
        1,
        Instruction::Mnemonic::BRMI,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{9, 7}}
    >;

    using Brne = Opcode<
        "BRNE",
        0b1111010000000001,
        1,
        Instruction::Mnemonic::BRNE,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{9, 7}}
    >;

    using Brpl = Opcode<
        "BRPL",
        0b1111010000000010,
        1,
        Instruction::Mnemonic::BRPL,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{9, 7}}
    >;

    using Brsh = Opcode<
        "BRSH",
        0b1111010000000000,
        1,
        Instruction::Mnemonic::BRSH,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{9, 7}}
    >;

    using Brtc = Opcode<
        "BRTC",
        0b1111010000000110,
        1,
        Instruction::Mnemonic::BRTC,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{9, 7}}
    >;

    using Brts = Opcode<
        "BRTS",
        0b1111000000000110,
        1,
        Instruction::Mnemonic::BRTS,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{9, 7}}
    >;

    using Brvc = Opcode<
        "BRVC",
        0b1111010000000011,
        1,
        Instruction::Mnemonic::BRVC,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{9, 7}}
    >;

    using Brvs = Opcode<
        "BRVS",
        0b1111000000000011,
        1,
        Instruction::Mnemonic::BRVS,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{9, 7}}
    >;

    using Bset = Opcode<
        "BSET",
        0b1001010000001000,
        1,
        Instruction::Mnemonic::BSET,
        false,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{6, 3}}
    >;

    using Bst = Opcode<
        "BST",
        0b1111101000000000,
        1,
        Instruction::Mnemonic::BST,
        false,
        RegisterParameter{{8, 5}},
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{2, 3}}
    >;

    using Call = Opcode<
        "CALL",
        0b10010100000011100000000000000000,
        2,
        Instruction::Mnemonic::CALL,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {24, 5},
            {16, 17},
        })}
    >;

    using Cbi = Opcode<
        "CBI",
        0b1001100000000000,
        1,
        Instruction::Mnemonic::CBI,
        false,
        std::nullopt,
        RegisterParameter{{7, 5}},
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{2, 3}}
    >;

    using Cbr = Opcode<
        "CBR",
        0b0111000000000000,
        1,
        Instruction::Mnemonic::CBR,
        false,
        std::nullopt,
        RegisterParameter{{7, 4}},
        InstructionParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {11, 4},
            {3, 4},
        })}
    >;

    using Clc = Opcode<
        "CLC",
        0b1001010010001000,
        1,
        Instruction::Mnemonic::CLC,
        false
    >;

    using Clh = Opcode<
        "CLH",
        0b1001010011011000,
        1,
        Instruction::Mnemonic::CLH,
        false
    >;

    using Cli = Opcode<
        "CLI",
        0b1001010011111000,
        1,
        Instruction::Mnemonic::CLI,
        false
    >;

    using Cln = Opcode<
        "CLN",
        0b1001010010101000,
        1,
        Instruction::Mnemonic::CLN,
        false
    >;

    using Clr = Opcode<
        "CLR",
        0b0010010000000000,
        1,
        Instruction::Mnemonic::CLR,
        false,
        std::nullopt,
        RegisterParameter{{9, 10}}
    >;

    using Cls = Opcode<
        "CLS",
        0b1001010011001000,
        1,
        Instruction::Mnemonic::CLS,
        false
    >;

    using Clt = Opcode<
        "CLT",
        0b1001010011101000,
        1,
        Instruction::Mnemonic::CLT,
        false
    >;

    using Clv = Opcode<
        "CLV",
        0b1001010010111000,
        1,
        Instruction::Mnemonic::CLV,
        false
    >;

    using Clz = Opcode<
        "CLZ",
        0b1001010010011000,
        1,
        Instruction::Mnemonic::CLZ,
        false
    >;

    using Com = Opcode<
        "COM",
        0b1001010000000000,
        1,
        Instruction::Mnemonic::COM,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using Cp = Opcode<
        "CP",
        0b0001010000000000,
        1,
        Instruction::Mnemonic::CP,
        false,
        RegisterParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {9, 1},
            {3, 4},
        })},
        RegisterParameter{{8, 5}}
    >;

    using Cpc = Opcode<
        "CPC",
        0b0000010000000000,
        1,
        Instruction::Mnemonic::CPC,
        false,
        RegisterParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {9, 1},
            {3, 4},
        })},
        RegisterParameter{{8, 5}}
    >;

    using Cpi = Opcode<
        "CPI",
        0b0011000000000000,
        1,
        Instruction::Mnemonic::CPI,
        false,
        RegisterParameter{{7, 4}},
        std::nullopt,
        InstructionParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {11, 4},
            {3, 4},
        })}
    >;

    using Cpse = Opcode<
        "CPSE",
        0b0001000000000000,
        1,
        Instruction::Mnemonic::CPSE,
        true,
        RegisterParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {9, 1},
            {3, 4},
        })},
        RegisterParameter{{8, 5}},
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        true
    >;

    using Dec = Opcode<
        "DEC",
        0b1001010000001010,
        1,
        Instruction::Mnemonic::DEC,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using Des = Opcode<
        "DES",
        0b1001010000001011,
        1,
        Instruction::Mnemonic::DES,
        false,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{7, 4}}
    >;

    using Eicall = Opcode<
        "EICALL",
        0b1001010100011001,
        1,
        Instruction::Mnemonic::EICALL,
        true
    >;

    using Eijmp = Opcode<
        "EIJMP",
        0b1001010000011001,
        1,
        Instruction::Mnemonic::EIJMP,
        true
    >;

    using Elpm1 = Opcode<
        "ELPM",
        0b1001010111011000,
        1,
        Instruction::Mnemonic::ELPM,
        false
    >;

    using Elpm2 = Opcode<
        "ELPM",
        0b1001000000000110,
        1,
        Instruction::Mnemonic::ELPM,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using Elpm3 = Opcode<
        "ELPM",
        0b1001000000000111,
        1,
        Instruction::Mnemonic::ELPM,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using Eor = Opcode<
        "EOR",
        0b0010010000000000,
        1,
        Instruction::Mnemonic::EOR,
        false,
        RegisterParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {9, 1},
            {3, 4},
        })},
        RegisterParameter{{8, 5}}
    >;

    using Fmul = Opcode<
        "FMUL",
        0b0000001100001000,
        1,
        Instruction::Mnemonic::FMUL,
        false,
        RegisterParameter{{2, 3}},
        RegisterParameter{{6, 3}}
    >;

    using Fmuls = Opcode<
        "FMULS",
        0b0000001110000000,
        1,
        Instruction::Mnemonic::FMULS,
        false,
        RegisterParameter{{2, 3}},
        RegisterParameter{{6, 3}}
    >;

    using Fmulsu = Opcode<
        "FMULSU",
        0b0000001110001000,
        1,
        Instruction::Mnemonic::FMULSU,
        false,
        RegisterParameter{{2, 3}},
        RegisterParameter{{6, 3}}
    >;

    using Icall = Opcode<
        "ICALL",
        0b1001010100001001,
        1,
        Instruction::Mnemonic::ICALL,
        true
    >;

    using Ijmp = Opcode<
        "IJMP",
        0b1001010000001001,
        1,
        Instruction::Mnemonic::IJMP,
        true
    >;

    using In = Opcode<
        "IN",
        0b1011000000000000,
        1,
        Instruction::Mnemonic::IN,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}},
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {10, 2},
            {3, 4}
        })}
    >;

    using Inc = Opcode<
        "INC",
        0b1001010000000011,
        1,
        Instruction::Mnemonic::INC,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using Jmp = Opcode<
        "JMP",
        0b10010100000011000000000000000000,
        2,
        Instruction::Mnemonic::JMP,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {24, 5},
            {16, 17}
        })}
    >;

    using Lac = Opcode<
        "LAC",
        0b1001001000000110,
        1,
        Instruction::Mnemonic::LAC,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using Las = Opcode<
        "LAS",
        0b1001001000000101,
        1,
        Instruction::Mnemonic::LAS,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using Lat = Opcode<
        "LAT",
        0b1001001000000111,
        1,
        Instruction::Mnemonic::LAT,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using LdX1 = Opcode<
        "LD",
        0b1001000000001100,
        1,
        Instruction::Mnemonic::LD,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using LdX2 = Opcode<
        "LD",
        0b1001000000001101,
        1,
        Instruction::Mnemonic::LD,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using LdX3 = Opcode<
        "LD",
        0b1001000000001110,
        1,
        Instruction::Mnemonic::LD,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using LdY1 = Opcode<
        "LD",
        0b1000000000001000,
        1,
        Instruction::Mnemonic::LD,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using LdY2 = Opcode<
        "LD",
        0b1001000000001001,
        1,
        Instruction::Mnemonic::LD,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using LdY3 = Opcode<
        "LD",
        0b1001000000001010,
        1,
        Instruction::Mnemonic::LD,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using LddY = Opcode<
        "LDD",
        0b1000000000001000,
        1,
        Instruction::Mnemonic::LDD,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}},
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {13, 1},
            {11, 2},
            {2, 3}
        })}
    >;

    using LdZ1 = Opcode<
        "LD",
        0b1000000000000000,
        1,
        Instruction::Mnemonic::LD,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using LdZ2 = Opcode<
        "LD",
        0b1001000000000001,
        1,
        Instruction::Mnemonic::LD,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using LdZ3 = Opcode<
        "LD",
        0b1001000000000010,
        1,
        Instruction::Mnemonic::LD,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using LddZ = Opcode<
        "LDD",
        0b1000000000000000,
        1,
        Instruction::Mnemonic::LDD,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}},
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {13, 1},
            {11, 2},
            {2, 3}
        })}
    >;

    using Ldi = Opcode<
        "LDI",
        0b1110000000000000,
        1,
        Instruction::Mnemonic::LDI,
        false,
        std::nullopt,
        RegisterParameter{16, false, {7, 4}},
        InstructionParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {11, 4},
            {3, 4}
        })}
    >;

    using Lds1 = Opcode<
        "LDS",
        0b10010000000000000000000000000000,
        2,
        Instruction::Mnemonic::LDS,
        false,
        std::nullopt,
        RegisterParameter{{24, 5}},
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{15, 16}}
    >;

    using Lds2 = Opcode<
        "LDS",
        0b1010000000000000,
        1,
        Instruction::Mnemonic::LDS,
        false,
        std::nullopt,
        RegisterParameter{{7, 4}},
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {10, 3},
            {3, 4}
        })}
    >;

    using Lpm1 = Opcode<
        "LPM",
        0b1001010111001000,
        1,
        Instruction::Mnemonic::LPM,
        false
    >;

    using Lpm2 = Opcode<
        "LPM",
        0b1001000000000100,
        1,
        Instruction::Mnemonic::LPM,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using Lpm3 = Opcode<
        "LPM",
        0b1001000000000101,
        1,
        Instruction::Mnemonic::LPM,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using Lsl = Opcode<
        "LSL",
        0b0000110000000000,
        1,
        Instruction::Mnemonic::LSL,
        false,
        std::nullopt,
        RegisterParameter{{9, 10}}
    >;

    using Lsr = Opcode<
        "LSR",
        0b1001010000000110,
        1,
        Instruction::Mnemonic::LSR,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using Mov = Opcode<
        "MOV",
        0b0010110000000000,
        1,
        Instruction::Mnemonic::MOV,
        false,
        RegisterParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {9, 1},
            {3, 4},
        })},
        RegisterParameter{{8, 5}}
    >;

    using Movw = Opcode<
        "MOVW",
        0b0000000100000000,
        1,
        Instruction::Mnemonic::MOVW,
        false,
        RegisterParameter{0, true, {3, 4}},
        RegisterParameter{0, true, {7, 4}}
    >;

    using Mul = Opcode<
        "MUL",
        0b1001110000000000,
        1,
        Instruction::Mnemonic::MUL,
        false,
        RegisterParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {9, 1},
            {3, 4},
        })},
        RegisterParameter{{8, 5}}
    >;

    using Muls = Opcode<
        "MULS",
        0b0000001000000000,
        1,
        Instruction::Mnemonic::MULS,
        false,
        RegisterParameter{{3, 4}},
        RegisterParameter{{7, 4}}
    >;

    using Mulsu = Opcode<
        "MULSU",
        0b0000001100000000,
        1,
        Instruction::Mnemonic::MULSU,
        false,
        RegisterParameter{{2, 3}},
        RegisterParameter{{6, 3}}
    >;

    using Neg = Opcode<
        "NEG",
        0b1001010000000001,
        1,
        Instruction::Mnemonic::NEG,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using Nop = Opcode<
        "NOP",
        0b0000000000000000,
        1,
        Instruction::Mnemonic::NOP,
        false
    >;

    using Or = Opcode<
        "OR",
        0b0010100000000000,
        1,
        Instruction::Mnemonic::OR,
        false,
        RegisterParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {9, 1},
            {3, 4},
        })},
        RegisterParameter{{8, 5}}
    >;

    using Ori = Opcode<
        "ORI",
        0b0110000000000000,
        1,
        Instruction::Mnemonic::ORI,
        false,
        std::nullopt,
        RegisterParameter{{7, 4}},
        InstructionParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {11, 4},
            {3, 4},
        })}
    >;

    using Out = Opcode<
        "OUT",
        0b1011100000000000,
        1,
        Instruction::Mnemonic::OUT,
        false,
        RegisterParameter{{8, 5}},
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {10, 2},
            {3, 4},
        })}
    >;

    using Pop = Opcode<
        "POP",
        0b1001000000001111,
        1,
        Instruction::Mnemonic::POP,
        false,
        RegisterParameter{{8, 5}}
    >;

    using Push = Opcode<
        "PUSH",
        0b1001001000001111,
        1,
        Instruction::Mnemonic::PUSH,
        false,
        RegisterParameter{{8, 5}}
    >;

    using Rcall = Opcode<
        "RCALL",
        0b1101000000000000,
        1,
        Instruction::Mnemonic::RCALL,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{11, 12}}
    >;

    using Ret = Opcode<
        "RET",
        0b1001010100001000,
        1,
        Instruction::Mnemonic::RET,
        true
    >;

    using Reti = Opcode<
        "RETI",
        0b1001010100011000,
        1,
        Instruction::Mnemonic::RETI,
        true
    >;

    using Rjmp = Opcode<
        "RJMP",
        0b1100000000000000,
        1,
        Instruction::Mnemonic::RJMP,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{11, 12}}
    >;

    using Rol = Opcode<
        "ROL",
        0b0001110000000000,
        1,
        Instruction::Mnemonic::ROL,
        false,
        std::nullopt,
        RegisterParameter{{9, 10}}
    >;

    using Ror = Opcode<
        "ROR",
        0b1001010000000111,
        1,
        Instruction::Mnemonic::ROR,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using Sbc = Opcode<
        "SBC",
        0b0000100000000000,
        1,
        Instruction::Mnemonic::SBC,
        false,
        RegisterParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {9, 1},
            {3, 4},
        })},
        RegisterParameter{{8, 5}}
    >;

    using Sbci = Opcode<
        "SBCI",
        0b0100000000000000,
        1,
        Instruction::Mnemonic::SBCI,
        false,
        std::nullopt,
        RegisterParameter{16, false, {7, 4}},
        InstructionParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {11, 4},
            {3, 4},
        })}
    >;

    using Sbi = Opcode<
        "SBI",
        0b1001101000000000,
        1,
        Instruction::Mnemonic::SBI,
        false,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{2, 3}},
        std::nullopt,
        InstructionParameter{{7, 5}}
    >;

    using Sbic = Opcode<
        "SBIC",
        0b1001100100000000,
        1,
        Instruction::Mnemonic::SBIC,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{2, 3}},
        std::nullopt,
        InstructionParameter{{7, 5}},
        std::nullopt,
        std::nullopt,
        true
    >;

    using Sbis = Opcode<
        "SBIS",
        0b1001101100000000,
        1,
        Instruction::Mnemonic::SBIS,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{2, 3}},
        std::nullopt,
        InstructionParameter{{7, 5}},
        std::nullopt,
        std::nullopt,
        true
    >;

    using Sbiw = Opcode<
        "SBIW",
        0b1001011100000000,
        1,
        Instruction::Mnemonic::SBIW,
        false,
        std::nullopt,
        RegisterParameter{24, true, {5, 2}},
        InstructionParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {7, 2},
            {3, 4},
        })}
    >;

    using Sbr = Opcode<
        "SBR",
        0b0110000000000000,
        1,
        Instruction::Mnemonic::SBR,
        false,
        std::nullopt,
        RegisterParameter{16, true, {7, 4}},
        InstructionParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {11, 4},
            {3, 4},
        })}
    >;

    using Sbrc = Opcode<
        "SBRC",
        0b1111110000000000,
        1,
        Instruction::Mnemonic::SBRC,
        true,
        RegisterParameter{{8, 5}},
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{2, 3}},
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        true
    >;

    using Sbrs = Opcode<
        "SBRS",
        0b1111111000000000,
        1,
        Instruction::Mnemonic::SBRS,
        true,
        RegisterParameter{{8, 5}},
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{2, 3}},
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        true
    >;

    using Sec = Opcode<
        "SEC",
        0b1001010000001000,
        1,
        Instruction::Mnemonic::SEC,
        false
    >;

    using Seh = Opcode<
        "SEH",
        0b1001010001011000,
        1,
        Instruction::Mnemonic::SEH,
        false
    >;

    using Sei = Opcode<
        "SEI",
        0b1001010001111000,
        1,
        Instruction::Mnemonic::SEI,
        false
    >;

    using Sen = Opcode<
        "SEN",
        0b1001010000101000,
        1,
        Instruction::Mnemonic::SEN,
        false
    >;

    using Ser = Opcode<
        "SER",
        0b1110111100001111,
        1,
        Instruction::Mnemonic::SER,
        false,
        std::nullopt,
        RegisterParameter{{7, 4}}
    >;

    using Ses = Opcode<
        "SES",
        0b1001010001001000,
        1,
        Instruction::Mnemonic::SES,
        false
    >;

    using Set = Opcode<
        "SET",
        0b1001010001101000,
        1,
        Instruction::Mnemonic::SET,
        false
    >;

    using Sev = Opcode<
        "SEV",
        0b1001010000111000,
        1,
        Instruction::Mnemonic::SEV,
        false
    >;

    using Sez = Opcode<
        "SEZ",
        0b1001010000011000,
        1,
        Instruction::Mnemonic::SEZ,
        false
    >;

    using Sleep = Opcode<
        "SLEEP",
        0b1001010110001000,
        1,
        Instruction::Mnemonic::SLEEP,
        false
    >;

    using Spm1 = Opcode<
        "SPM",
        0b1001010111101000,
        1,
        Instruction::Mnemonic::SPM,
        false
    >;

    using Spm2 = Opcode<
        "SPM",
        0b1001010111111000,
        1,
        Instruction::Mnemonic::SPM,
        false
    >;

    using StX1 = Opcode<
        "ST",
        0b1001001000001100,
        1,
        Instruction::Mnemonic::ST,
        false,
        RegisterParameter{{8, 5}}
    >;

    using StX2 = Opcode<
        "ST",
        0b1001001000001101,
        1,
        Instruction::Mnemonic::ST,
        false,
        RegisterParameter{{8, 5}}
    >;

    using StX3 = Opcode<
        "ST",
        0b1001001000001110,
        1,
        Instruction::Mnemonic::ST,
        false,
        RegisterParameter{{8, 5}}
    >;

    using StY1 = Opcode<
        "ST",
        0b1000001000001000,
        1,
        Instruction::Mnemonic::ST,
        false,
        RegisterParameter{{8, 5}}
    >;

    using StY2 = Opcode<
        "ST",
        0b1001001000001001,
        1,
        Instruction::Mnemonic::ST,
        false,
        RegisterParameter{{8, 5}}
    >;

    using StY3 = Opcode<
        "ST",
        0b1001001000001010,
        1,
        Instruction::Mnemonic::ST,
        false,
        RegisterParameter{{8, 5}}
    >;

    using StdY = Opcode<
        "STD",
        0b1000001000001000,
        1,
        Instruction::Mnemonic::STD,
        false,
        RegisterParameter{{8, 5}},
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {13, 1},
            {11, 2},
            {2, 3}
        })}
    >;

    using StZ1 = Opcode<
        "ST",
        0b1000001000000000,
        1,
        Instruction::Mnemonic::ST,
        false,
        RegisterParameter{{8, 5}}
    >;

    using StZ2 = Opcode<
        "ST",
        0b1001001000000001,
        1,
        Instruction::Mnemonic::ST,
        false,
        RegisterParameter{{8, 5}}
    >;

    using StZ3 = Opcode<
        "ST",
        0b1001001000000010,
        1,
        Instruction::Mnemonic::ST,
        false,
        RegisterParameter{{8, 5}}
    >;

    using StdZ = Opcode<
        "STD",
        0b1000001000000000,
        1,
        Instruction::Mnemonic::STD,
        false,
        RegisterParameter{{8, 5}},
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {13, 1},
            {11, 2},
            {2, 3}
        })}
    >;

    using Sts1 = Opcode<
        "STS",
        0b10010010000000000000000000000000,
        2,
        Instruction::Mnemonic::STS,
        false,
        RegisterParameter{{24, 5}},
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        InstructionParameter{{15, 16}}
    >;

    using Sts2 = Opcode<
        "STS",
        0b1010100000000000,
        1,
        Instruction::Mnemonic::STS,
        false,
        RegisterParameter{{7, 4}},
        std::nullopt,
        /*
         * TODO:
         * This is supposed to be a 7-bit data space address, but it requires additional parsing to be converted to an
         * actual address. For that reason, we just treat it as a constant data param.
         *
         * We're not actually making use of data space addresses ATM, so it's not a big deal for now. Will sort it
         * later.
         */
        InstructionParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {10, 3},
            {3, 4},
        })}
    >;

    using Sub = Opcode<
        "SUB",
        0b0001100000000000,
        1,
        Instruction::Mnemonic::SUB,
        false,
        RegisterParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {9, 1},
            {3, 4},
        })},
        RegisterParameter{{8, 5}}
    >;

    using Subi = Opcode<
        "SUBI",
        0b0101000000000000,
        1,
        Instruction::Mnemonic::SUBI,
        false,
        std::nullopt,
        RegisterParameter{{7, 4}},
        RegisterParameter{std::to_array<Services::BitsetService::BitFieldRange>({
            {11, 4},
            {3, 4},
        })}
    >;

    using Swap = Opcode<
        "SWAP",
        0b1001010000000010,
        1,
        Instruction::Mnemonic::SWAP,
        false,
        std::nullopt,
        RegisterParameter{{8, 5}}
    >;

    using Tst = Opcode<
        "TST",
        0b0010000000000000,
        1,
        Instruction::Mnemonic::TST,
        false,
        std::nullopt,
        RegisterParameter{{9, 10}}
    >;

    /*
     * So apparently, some AVRs treat the undefined 0xFFFF opcode as 0xFFF7 (SBRS R31, 7).
     *
     * See https://www.avrfreaks.net/s/topic/a5C3l000000UL4NEAW/t094785
     *
     * For this reason, we have to define it here.
     */
    using UndefinedOrErased = Opcode<
        "????",
        0b1111111111111111,
        1,
        Instruction::Mnemonic::UNDEFINED,
        true,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        true
    >;

    using Wdr = Opcode<
        "WDR",
        0b1001010110101000,
        1,
        Instruction::Mnemonic::WDR,
        false
    >;

    using Xch = Opcode<
        "XCH",
        0b1001001000000100,
        1,
        Instruction::Mnemonic::XCH,
        false,
        RegisterParameter{{8, 5}}
    >;
}
