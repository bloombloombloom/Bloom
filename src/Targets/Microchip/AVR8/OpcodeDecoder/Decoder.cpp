#include "Decoder.hpp"

#include <iterator>

#include "Opcodes.hpp"

#include "Exceptions/DecodeFailure.hpp"

namespace Targets::Microchip::Avr8::OpcodeDecoder
{
    Decoder::InstructionMapping Decoder::decode(
        Targets::TargetMemoryAddress startByteAddress,
        const TargetMemoryBuffer& data,
        bool throwOnFailure
    ) {
        auto output = Decoder::InstructionMapping{};

        static const auto decoders = Decoder::opcodeDecoders();

        auto instructionByteAddress = startByteAddress;
        auto dataIt = data.begin();
        const auto dataEndIt = data.end();

        while (std::distance(dataIt, dataEndIt) >= 2) {
            auto opcodeMatched = false;

            for (const auto& decoder : decoders) {
                auto instruction = decoder(dataIt, dataEndIt);

                if (instruction.has_value()) {
                    const auto instructionSize = instruction->byteSize;
                    output.emplace(instructionByteAddress, std::move(*instruction));

                    dataIt += instructionSize;
                    instructionByteAddress += instructionSize;
                    opcodeMatched = true;
                    break;
                }
            }

            if (!opcodeMatched) {
                if (throwOnFailure) {
                    throw Exceptions::DecodeFailure{
                        instructionByteAddress,
                        static_cast<std::uint32_t>(*(dataIt + 1) << 8) | *dataIt
                    };
                }

                output.emplace(instructionByteAddress, std::nullopt);

                dataIt += 2;
                instructionByteAddress += 2;
            }
        }

        return output;
    }

    Decoder::OpcodeDecoders Decoder::opcodeDecoders() {
        /*
         * The decoders will be used in the order given here.
         *
         * I've used the same order that is used in the AVR implementation of GDB.
         */
        return Decoder::OpcodeDecoders{
            std::bind(&Opcodes::UndefinedOrErased::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Clc::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Clh::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Cli::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Cln::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Cls::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Clt::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Clv::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Clz::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Sec::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Seh::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Sei::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Sen::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Ses::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Set::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Sev::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Sez::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Bclr::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Bset::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Icall::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Ijmp::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Lpm1::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Lpm2::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Lpm3::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Elpm1::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Elpm2::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Elpm3::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Nop::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Ret::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Reti::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Sleep::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Break::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Wdr::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Spm1::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Spm2::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Adc::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Add::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::And::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Cp::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Cpc::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Cpse::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Eor::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Mov::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Mul::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Or::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Sbc::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Sub::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Clr::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Lsl::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Rol::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Tst::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Andi::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Cbr::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Ldi::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Ser::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Ori::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Sbr::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Cpi::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Sbci::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Subi::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Sbrc::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Sbrs::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Bld::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Bst::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::In::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Out::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Adiw::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Sbiw::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Cbi::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Sbi::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Sbic::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Sbis::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Brcc::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Brcs::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Breq::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Brge::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Brhc::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Brhs::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Brid::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Brie::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Brlo::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Brlt::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Brmi::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Brne::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Brpl::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Brsh::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Brtc::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Brts::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Brvc::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Brvs::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Brbc::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Brbs::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Rcall::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Rjmp::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Call::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Jmp::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Asr::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Com::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Dec::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Inc::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Lsr::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Neg::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Pop::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Push::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Ror::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Swap::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Xch::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Las::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Lac::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Lat::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Movw::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Muls::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Mulsu::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Fmul::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Fmuls::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Fmulsu::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Sts1::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Sts2::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Lds1::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Lds2::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::LddY::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::LddZ::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::LdX1::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::LdX2::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::LdX3::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::LdY1::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::LdY2::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::LdY3::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::LdZ1::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::LdZ2::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::LdZ3::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::StdY::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::StdZ::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::StX1::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::StX2::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::StX3::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::StY1::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::StY2::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::StY3::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::StZ1::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::StZ2::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::StZ3::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Eicall::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Eijmp::decode, std::placeholders::_1, std::placeholders::_2),
            std::bind(&Opcodes::Des::decode, std::placeholders::_1, std::placeholders::_2),
        };
    }
}
