#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <iterator>
#include <array>
#include <cassert>
#include <optional>

#include "Instruction.hpp"
#include "src/Helpers/FixedString.hpp"
#include "src/Targets/TargetMemory.hpp"
#include "src/Services/BitsetService.hpp"

namespace Targets::Microchip::Avr8::OpcodeDecoder
{
    struct InstructionParameterBase
    {};

    template<std::size_t bitFieldRangeCount = 1>
    struct InstructionParameter: public InstructionParameterBase
    {
        /*
         * Not all instruction parameters are encoded in a consecutive manner. Some are scattered all over the place.
         *
         * For example, the CALL instruction accepts a single address parameter, but the 32-bit opcode looks
         * like this: 0b1001010kkkkk111kkkkkkkkkkkkkkkkk. Where the 'k's are the address parameter bits, which are not
         * encoded consecutively.
         *
         * There is one variation of the LD instruction that is even worse, where a single parameter is scattered in
         * three different locations of the opcode.
         *
         * Because parameter bits can be scattered, we must accept a set of BitFieldRanges, as opposed to just one.
         */
        const std::array<Services::BitsetService::BitFieldRange, bitFieldRangeCount> bitFieldRanges;

        const std::uint8_t length;
        const std::uint32_t mask;

        constexpr InstructionParameter(
            const std::array<Services::BitsetService::BitFieldRange, bitFieldRangeCount>& bitFieldRanges
        )
            : bitFieldRanges(bitFieldRanges)
            , length(Services::BitsetService::totalBitRangeLength(bitFieldRanges))
            , mask(Services::BitsetService::setBitField(std::uint32_t{0}, bitFieldRanges))
        {}

        constexpr InstructionParameter(const Services::BitsetService::BitFieldRange& bitFieldRange)
            : InstructionParameter(
                std::array<Services::BitsetService::BitFieldRange, bitFieldRangeCount>({bitFieldRange})
            )
        {}
    };

    template<std::size_t bitFieldRangeCount = 1>
    struct RegisterParameter: public InstructionParameter<bitFieldRangeCount>
    {
        const std::uint8_t offset = 0;
        const bool pair = false;

        constexpr RegisterParameter(
            std::uint8_t offset,
            bool pair,
            const std::array<Services::BitsetService::BitFieldRange, bitFieldRangeCount>& bitFieldRanges
        )
            : InstructionParameter<bitFieldRangeCount>(bitFieldRanges)
            , offset(offset)
            , pair(pair)
        {}

        constexpr RegisterParameter(
            std::uint8_t offset,
            bool pair,
            const Services::BitsetService::BitFieldRange& bitFieldRange
        )
            : InstructionParameter<bitFieldRangeCount>(bitFieldRange)
            , offset(offset)
            , pair(pair)
        {}

        constexpr RegisterParameter(
            const std::array<Services::BitsetService::BitFieldRange, bitFieldRangeCount>& bitFieldRanges
        )
            : InstructionParameter<bitFieldRangeCount>(bitFieldRanges)
        {}

        constexpr RegisterParameter(const Services::BitsetService::BitFieldRange& bitFieldRange)
            : InstructionParameter<bitFieldRangeCount>(bitFieldRange)
        {}
    };

    template<typename>
    struct IsRegisterParam: std::false_type {};

    template<std::size_t bitFieldRangeCount>
    struct IsRegisterParam<RegisterParameter<bitFieldRangeCount>>: std::true_type {};

    template<typename ParamType>
        requires (std::is_same_v<ParamType, std::nullopt_t> || std::is_base_of_v<InstructionParameterBase, ParamType>)
    class OptionalInstructionParameter
    {
    public:
        ParamType value;

        constexpr OptionalInstructionParameter(ParamType value)
            : value(value)
        {};

        static constexpr bool hasValue() {
            if constexpr (std::is_base_of_v<InstructionParameterBase, ParamType>) {
                return true;

            } else {
                return false;
            }
        }
    };

    /**
     * Base for InstructionParameterMask(ParamType param, ParamPackType... params).
     */
    template<typename ParamType>
    static constexpr std::uint32_t InstructionParameterMask(ParamType param) {
        if constexpr (!decltype(param)::hasValue()) {
            return 0;

        } else {
            return param.value.mask;
        }
    }

    /**
     * Returns a combined (ORed) mask of the given params.
     */
    template<typename ParamType, typename... ParamPackType>
    static constexpr std::uint32_t InstructionParameterMask(ParamType param, ParamPackType... params) {
        if constexpr (!decltype(param)::hasValue()) {
            return InstructionParameterMask(params...);

        } else {
            return param.value.mask | InstructionParameterMask(params...);
        }
    }

    /**
     * This template class is used to describe a particular AVR8 instruction opcode.
     *
     * It provides decoding abilities, as well as compile-time error checks.
     *
     * @tparam instructionName
     *  The name of the AVR8 instruction, in the form of a string literal.
     *
     * @tparam expectedOpcode
     *  The expected opcode of the instruction, in the form of a binary literal, with all parameter bits cleared (which
     *  is enforced by template constraints).
     *
     *  For example, consider the AVR8 OR instruction opcode: 0b001010rdddddrrrr, where 'r' and 'd' are source and
     *  destination register parameters, respectively. The expectedOpcode value would be 0b0010100000000000.
     *
     * @tparam wordSize
     *  The word size of the instruction. AVR8 instructions are either 1 or 2 words.
     *
     * @tparam mnemonic
     *  The instruction's mnemonic.
     *
     * @tparam canChangeProgramFlow
     *  Whether the instruction **can** change program flow.
     *
     *  If the instruction can change the program counter to anything other than PC + n, where n is the word size of
     *  the instruction, then this should be set to true.
     *
     * @tparam sourceRegisterParameter
     *  If the instruction encodes a source register parameter (like the OR instruction), this should be provided here.
     *  Otherwise, std::nullopt.
     *
     *  Register parameters have some additional information we need (e.g. offsets, or if they should be interpreted as
     *  register pairs). For this reason, register parameters should be provided as instances of RegisterParameter (as
     *  opposed to instances of InstructionParameter).
     *
     * @tparam destinationRegisterParameter
     *  If the instruction encodes a destination register parameter (like the OR instruction), this should be provided
     *  here. Otherwise, std::nullopt.
     *
     *  This should be an instance of RegisterParameter. See above.
     *
     * @tparam dataParameter
     *  If the instruction encodes a constant data parameter (like the ADIW instruction), this should be provided here.
     *  Otherwise, std::nullopt.
     *
     * @tparam programAddressParameter
     *  If the instruction encodes a constant program word address parameter (like the CALL instruction), this should
     *  be provided here. Otherwise, std::nullopt.
     *
     *  If this parameter is provided, Opcode::canChangeProgramFlow must be set to true. This is enforced by template
     *  constraints.
     *
     * @tparam programAddressOffsetParameter
     *  If the instruction encodes a constant program word address offset parameter (like the RJMP instruction), this
     *  should be provided here. Otherwise, std::nullopt.
     *
     *  If this parameter is provided, Opcode::canChangeProgramFlow must be set to true. This is enforced by template
     *  constraints.
     *
     * @tparam registerBitPositionParameter
     *  If the instruction encodes a register bit position parameter (like the SBIS instruction), this should be
     *  provided here. Otherwise, std::nullopt.
     *
     * @tparam statusRegisterBitPositionParameter
     *  If the instruction encodes a status register bit position parameter (like the BRBS instruction), this should be
     *  provided here. Otherwise, std::nullopt.
     *
     * @tparam ioSpaceAddressParameter
     *  If the instruction encodes an I/O space parameter (like the IN instruction), this should be provided here.
     *  Otherwise, std::nullopt.
     *
     * @tparam dataSpaceAddressParameter
     *  If the instruction encodes a data space address parameter (like the STS instruction), this should be provided
     *  here. Otherwise, std::nullopt.
     *
     * @tparam displacementParameter
     *  If the instruction encodes a displacement parameter (like the STD instruction), this should be provided here.
     *  Otherwise, std::nullopt.
     *
     * @tparam canSkipNextInstruction
     *  Whether the instruction can skip the following instruction.
     *
     *  If this is set to true, Opcode::canChangeProgramFlow must also be set to true. This is enforced by template
     *  constraints.
     */
    template <
        FixedString instructionName,
        std::uint32_t expectedOpcode,
        int wordSize,
        Instruction::Mnemonic mnemonic,
        bool canChangeProgramFlow,
        OptionalInstructionParameter sourceRegisterParameter = std::nullopt,
        OptionalInstructionParameter destinationRegisterParameter = std::nullopt,
        OptionalInstructionParameter dataParameter = std::nullopt,
        OptionalInstructionParameter programAddressParameter = std::nullopt,
        OptionalInstructionParameter programAddressOffsetParameter = std::nullopt,
        OptionalInstructionParameter registerBitPositionParameter = std::nullopt,
        OptionalInstructionParameter statusRegisterBitPositionParameter = std::nullopt,
        OptionalInstructionParameter ioSpaceAddressParameter = std::nullopt,
        OptionalInstructionParameter dataSpaceAddressParameter = std::nullopt,
        OptionalInstructionParameter displacementParameter = std::nullopt,
        bool canSkipNextInstruction = false
    >
        requires
            (wordSize == 1 || wordSize == 2)
            && (wordSize == 2 || expectedOpcode <= 0x0000FFFF)
            /*
             * All parameter bits in expectedOpcode should be cleared. We enforce this here.
             */
            && (
                !decltype(sourceRegisterParameter)::hasValue()
                || ((expectedOpcode & sourceRegisterParameter.value.mask) == 0)
            )
            && (
                !decltype(destinationRegisterParameter)::hasValue()
                || ((expectedOpcode & destinationRegisterParameter.value.mask) == 0)
            )
            && (
                !decltype(dataParameter)::hasValue()
                || ((expectedOpcode & dataParameter.value.mask) == 0)
            )
            && (
                !decltype(programAddressParameter)::hasValue()
                || ((expectedOpcode & programAddressParameter.value.mask) == 0)
            )
            && (
                !decltype(programAddressOffsetParameter)::hasValue()
                || ((expectedOpcode & programAddressOffsetParameter.value.mask) == 0)
            )
            && (
                !decltype(registerBitPositionParameter)::hasValue()
                || ((expectedOpcode & registerBitPositionParameter.value.mask) == 0)
            )
            && (
                !decltype(statusRegisterBitPositionParameter)::hasValue()
                || ((expectedOpcode & statusRegisterBitPositionParameter.value.mask) == 0)
            )
            && (
                !decltype(ioSpaceAddressParameter)::hasValue()
                || ((expectedOpcode & ioSpaceAddressParameter.value.mask) == 0)
            )
            && (
                !decltype(dataSpaceAddressParameter)::hasValue()
                || ((expectedOpcode & dataSpaceAddressParameter.value.mask) == 0)
            )
            && (
                !decltype(displacementParameter)::hasValue()
                || ((expectedOpcode & displacementParameter.value.mask) == 0)
            )
            /*
             * We need additional info for register parameters, which should be provided via the RegisterParameter type.
             */
            && (
                !decltype(sourceRegisterParameter)::hasValue()
                || IsRegisterParam<decltype(sourceRegisterParameter.value)>::value
            )
            && (
                !decltype(destinationRegisterParameter)::hasValue()
                || IsRegisterParam<decltype(destinationRegisterParameter.value)>::value
            )
            /*
             * Parameters should not collide (share opcode bits) with others. We detect any collisions here.
             */
            && (
                !decltype(sourceRegisterParameter)::hasValue()
                || (InstructionParameterMask(
                    destinationRegisterParameter,
                    dataParameter,
                    programAddressParameter,
                    programAddressOffsetParameter,
                    registerBitPositionParameter,
                    statusRegisterBitPositionParameter,
                    ioSpaceAddressParameter,
                    dataSpaceAddressParameter,
                    displacementParameter
                ) & sourceRegisterParameter.value.mask) == 0
            )
            && (
                !decltype(destinationRegisterParameter)::hasValue()
                || (InstructionParameterMask(
                    sourceRegisterParameter,
                    dataParameter,
                    programAddressParameter,
                    programAddressOffsetParameter,
                    registerBitPositionParameter,
                    statusRegisterBitPositionParameter,
                    ioSpaceAddressParameter,
                    dataSpaceAddressParameter,
                    displacementParameter
                ) & destinationRegisterParameter.value.mask) == 0
            )
            && (
                !decltype(dataParameter)::hasValue()
                || (InstructionParameterMask(
                    sourceRegisterParameter,
                    destinationRegisterParameter,
                    programAddressParameter,
                    programAddressOffsetParameter,
                    registerBitPositionParameter,
                    statusRegisterBitPositionParameter,
                    ioSpaceAddressParameter,
                    dataSpaceAddressParameter,
                    displacementParameter
                ) & dataParameter.value.mask) == 0
            )
            && (
                !decltype(programAddressParameter)::hasValue()
                || (InstructionParameterMask(
                    sourceRegisterParameter,
                    destinationRegisterParameter,
                    dataParameter,
                    programAddressOffsetParameter,
                    registerBitPositionParameter,
                    statusRegisterBitPositionParameter,
                    ioSpaceAddressParameter,
                    dataSpaceAddressParameter,
                    displacementParameter
                ) & programAddressParameter.value.mask) == 0
            )
            && (
                !decltype(programAddressOffsetParameter)::hasValue()
                || (InstructionParameterMask(
                    sourceRegisterParameter,
                    destinationRegisterParameter,
                    dataParameter,
                    programAddressParameter,
                    registerBitPositionParameter,
                    statusRegisterBitPositionParameter,
                    ioSpaceAddressParameter,
                    dataSpaceAddressParameter,
                    displacementParameter
                ) & programAddressOffsetParameter.value.mask) == 0
            )
            && (
                !decltype(registerBitPositionParameter)::hasValue()
                || (InstructionParameterMask(
                    sourceRegisterParameter,
                    destinationRegisterParameter,
                    dataParameter,
                    programAddressParameter,
                    programAddressOffsetParameter,
                    statusRegisterBitPositionParameter,
                    ioSpaceAddressParameter,
                    dataSpaceAddressParameter,
                    displacementParameter
                ) & registerBitPositionParameter.value.mask) == 0
            )
            && (
                !decltype(statusRegisterBitPositionParameter)::hasValue()
                || (InstructionParameterMask(
                    sourceRegisterParameter,
                    destinationRegisterParameter,
                    dataParameter,
                    programAddressParameter,
                    programAddressOffsetParameter,
                    registerBitPositionParameter,
                    ioSpaceAddressParameter,
                    dataSpaceAddressParameter,
                    displacementParameter
                ) & statusRegisterBitPositionParameter.value.mask) == 0
            )
            && (
                !decltype(ioSpaceAddressParameter)::hasValue()
                || (InstructionParameterMask(
                    sourceRegisterParameter,
                    destinationRegisterParameter,
                    dataParameter,
                    programAddressParameter,
                    programAddressOffsetParameter,
                    registerBitPositionParameter,
                    statusRegisterBitPositionParameter,
                    dataSpaceAddressParameter,
                    displacementParameter
                ) & ioSpaceAddressParameter.value.mask) == 0
            )
            && (
                !decltype(dataSpaceAddressParameter)::hasValue()
                || (InstructionParameterMask(
                    sourceRegisterParameter,
                    destinationRegisterParameter,
                    dataParameter,
                    programAddressParameter,
                    programAddressOffsetParameter,
                    registerBitPositionParameter,
                    statusRegisterBitPositionParameter,
                    ioSpaceAddressParameter,
                    displacementParameter
                ) & dataSpaceAddressParameter.value.mask) == 0
            )
            && (
                !decltype(displacementParameter)::hasValue()
                || (InstructionParameterMask(
                    sourceRegisterParameter,
                    destinationRegisterParameter,
                    dataParameter,
                    programAddressParameter,
                    programAddressOffsetParameter,
                    registerBitPositionParameter,
                    statusRegisterBitPositionParameter,
                    ioSpaceAddressParameter,
                    dataSpaceAddressParameter
                ) & displacementParameter.value.mask) == 0
            )
            /*
             * Instructions that encode a program address/offset, or skip the next instruction, can change program flow
             * and should be marked as such. This is enforced here.
             */
            && (!decltype(programAddressParameter)::hasValue() || canChangeProgramFlow)
            && (!decltype(programAddressOffsetParameter)::hasValue() || canChangeProgramFlow)
            && (!canSkipNextInstruction || canChangeProgramFlow)
    class Opcode
    {
        using SelfType = Opcode<
            instructionName,
            expectedOpcode,
            wordSize,
            mnemonic,
            canChangeProgramFlow,
            sourceRegisterParameter,
            destinationRegisterParameter,
            dataParameter,
            programAddressParameter,
            programAddressOffsetParameter,
            registerBitPositionParameter,
            statusRegisterBitPositionParameter,
            ioSpaceAddressParameter,
            dataSpaceAddressParameter,
            displacementParameter,
            canSkipNextInstruction
        >;
        using OpcodeDataType = std::conditional_t<(wordSize > 1), std::uint32_t, std::uint16_t>;

        /**
         * We hold a single instance of the instruction name here, as a const static member, and then pass it by
         * reference to any new instances of the OpcodeDecoder::Instruction struct (which holds a const reference).
         */
        static const inline std::string name = instructionName.value;

    public:

        /**
         * Attempts to match and decode the first opcode that resides between the two iterators.
         *
         * @param dataBegin
         *  An iterator addressing some byte in the target's program memory. Keep in mind that AVR opcodes are stored
         *  in LSB form, so this iterator should address the least significant byte of the opcode.
         *
         * @param dataEnd
         *  An iterator addressing the byte at which decoding should stop, or just the end of the program memory.
         *
         * @return
         *  An instance of the OpcodeDecoder::Instruction struct, if the opcode was successfully matched and decoded.
         *  Otherwise, std::nullopt.
         */
        static std::optional<Instruction> decode(
            const Targets::TargetMemoryBuffer::const_iterator& dataBegin,
            const Targets::TargetMemoryBuffer::const_iterator& dataEnd
        ) {
            using Services::BitsetService;

            constexpr auto byteSize = wordSize * 2;
            if (std::distance(dataBegin, dataEnd) < byteSize) {
                // There isn't enough data to hold this instruction.
                return std::nullopt;
            }

            auto opcode = static_cast<OpcodeDataType>(*(dataBegin + 1) << 8 | *dataBegin);

            if constexpr (wordSize == 2) {
                opcode = (opcode << 16) | static_cast<OpcodeDataType>(*(dataBegin + 3) << 8 | *(dataBegin + 2));
            }

            if ((opcode & SelfType::opcodeMask()) != static_cast<OpcodeDataType>(expectedOpcode)) {
                // Opcode mismatch
                return std::nullopt;
            }

            auto output = Instruction{
                SelfType::name,
                opcode,
                byteSize,
                mnemonic,
                canChangeProgramFlow
            };

            if constexpr (decltype(sourceRegisterParameter)::hasValue()) {
                constexpr auto param = sourceRegisterParameter.value;

                static_assert(
                    param.length <= sizeof(decltype(Instruction::sourceRegister)::value_type) * 8,
                    "Invalid instruction sourceRegisterParameter - param length exceeds Instruction::sourceRegister size"
                );

                static_assert(
                    ((param.bitFieldRanges.begin())->startPosition + 1) >= param.length,
                    "Invalid instruction sourceRegisterParameter - parameter length exceeds available bits"
                );

                auto value = BitsetService::extractBitField(opcode, param.bitFieldRanges);

                if constexpr (param.pair) {
                    value = value * 2;
                }

                output.sourceRegister = static_cast<decltype(Instruction::sourceRegister)::value_type>(
                    value + param.offset
                );
            }

            if constexpr (decltype(destinationRegisterParameter)::hasValue()) {
                constexpr auto param = destinationRegisterParameter.value;

                static_assert(
                    ((param.bitFieldRanges.begin())->startPosition + 1) >= param.length,
                    "Invalid instruction destinationRegisterParameter - parameter length exceeds available bits"
                );

                auto value = BitsetService::extractBitField(opcode, param.bitFieldRanges);

                if constexpr (param.pair) {
                    value = value * 2;
                }

                output.destinationRegister = static_cast<decltype(Instruction::destinationRegister)::value_type>(
                    value + param.offset
                );
            }

            if constexpr (decltype(dataParameter)::hasValue()) {
                constexpr auto param = dataParameter.value;

                static_assert(
                    param.length <= sizeof(decltype(Instruction::data)::value_type) * 8,
                    "Invalid instruction dataParameter - param length exceeds Instruction::data size"
                );

                static_assert(
                    ((param.bitFieldRanges.begin())->startPosition + 1) >= param.length,
                    "Invalid instruction dataParameter - parameter length exceeds available bits"
                );

                output.data = static_cast<decltype(Instruction::data)::value_type>(
                    BitsetService::extractBitField(opcode, param.bitFieldRanges)
                );
            }

            if constexpr (decltype(programAddressParameter)::hasValue()) {
                constexpr auto param = programAddressParameter.value;

                static_assert(
                    param.length <= sizeof(decltype(Instruction::programWordAddress)::value_type) * 8,
                    "Invalid instruction programAddressParameter - param length exceeds "
                    "Instruction::destinationWordAddress size"
                );

                static_assert(
                    ((param.bitFieldRanges.begin())->startPosition + 1) >= param.length,
                    "Invalid instruction programAddressParameter - parameter length exceeds available bits"
                );

                output.programWordAddress = BitsetService::extractBitField(opcode, param.bitFieldRanges);
            }

            if constexpr (decltype(programAddressOffsetParameter)::hasValue()) {
                constexpr auto param = programAddressOffsetParameter.value;

                static_assert(
                    param.length <= sizeof(decltype(Instruction::programWordAddressOffset)::value_type) * 8,
                    "Invalid instruction programAddressOffsetParameter - param length exceeds "
                    "Instruction::destinationWordAddressOffset size"
                );

                static_assert(
                    ((param.bitFieldRanges.begin())->startPosition + 1) >= param.length,
                    "Invalid instruction programAddressOffsetParameter - param length exceeds available bits"
                );

                auto addressOffset = static_cast<std::int16_t>(
                    BitsetService::extractBitField(opcode, param.bitFieldRanges)
                );

                const auto signBitMask = static_cast<std::int16_t>(0x01 << (param.length - 1));
                if (addressOffset & signBitMask) {
                    // Sign extending required
                    addressOffset |= static_cast<std::int16_t>(-1LL) << param.length;
                }

                output.programWordAddressOffset = addressOffset;
            }

            if constexpr (decltype(registerBitPositionParameter)::hasValue()) {
                constexpr auto param = registerBitPositionParameter.value;

                static_assert(
                    param.length <= sizeof(decltype(Instruction::registerBitPosition)::value_type) * 8,
                    "Invalid instruction registerBitPositionParameter - param length exceeds "
                    "Instruction::registerBitPosition size"
                );

                static_assert(
                    ((param.bitFieldRanges.begin())->startPosition + 1) >= param.length,
                    "Invalid instruction registerBitPositionParameter - parameter length exceeds available bits"
                );

                output.registerBitPosition = static_cast<decltype(Instruction::registerBitPosition)::value_type>(
                    BitsetService::extractBitField(opcode, param.bitFieldRanges)
                );
            }

            if constexpr (decltype(statusRegisterBitPositionParameter)::hasValue()) {
                constexpr auto param = statusRegisterBitPositionParameter.value;

                static_assert(
                    param.length <= sizeof(decltype(Instruction::statusRegisterBitPosition)::value_type) * 8,
                    "Invalid instruction statusRegisterBitPositionParameter - param length exceeds "
                    "Instruction::statusRegisterBitPosition size"
                );

                static_assert(
                    ((param.bitFieldRanges.begin())->startPosition + 1) >= param.length,
                    "Invalid instruction statusRegisterBitPositionParameter - parameter length exceeds available bits"
                );

                output.statusRegisterBitPosition = static_cast<
                    decltype(Instruction::statusRegisterBitPosition)::value_type
                >(
                    BitsetService::extractBitField(opcode, param.bitFieldRanges)
                );
            }

            if constexpr (decltype(ioSpaceAddressParameter)::hasValue()) {
                constexpr auto param = ioSpaceAddressParameter.value;

                static_assert(
                    param.length <= sizeof(decltype(Instruction::ioSpaceAddress)::value_type) * 8,
                    "Invalid instruction ioSpaceAddressParameter - param length exceeds Instruction::ioSpaceAddress "
                    "size"
                );

                static_assert(
                    ((param.bitFieldRanges.begin())->startPosition + 1) >= param.length,
                    "Invalid instruction ioSpaceAddressParameter - parameter length exceeds available bits"
                );

                output.ioSpaceAddress = BitsetService::extractBitField(opcode, param.bitFieldRanges);
            }

            if constexpr (decltype(dataSpaceAddressParameter)::hasValue()) {
                constexpr auto param = dataSpaceAddressParameter.value;

                static_assert(
                    param.length <= sizeof(decltype(Instruction::dataSpaceAddress)::value_type) * 8,
                    "Invalid instruction dataSpaceAddressParameter - param length exceeds Instruction::dataSpaceAddress "
                    "size"
                );

                static_assert(
                    ((param.bitFieldRanges.begin())->startPosition + 1) >= param.length,
                    "Invalid instruction dataSpaceAddressParameter - parameter length exceeds available bits"
                );

                output.dataSpaceAddress = BitsetService::extractBitField(opcode, param.bitFieldRanges);
            }

            if constexpr (decltype(displacementParameter)::hasValue()) {
                constexpr auto param = displacementParameter.value;

                static_assert(
                    param.length <= sizeof(decltype(Instruction::displacement)::value_type) * 8,
                    "Invalid instruction displacementParameter - param length exceeds Instruction::displacement size"
                );

                static_assert(
                    ((param.bitFieldRanges.begin())->startPosition + 1) >= param.length,
                    "Invalid instruction displacementParameter - parameter length exceeds available bits"
                );

                output.displacement = BitsetService::extractBitField(opcode, param.bitFieldRanges);
            }

            output.canSkipNextInstruction = canSkipNextInstruction;

            return output;
        }

    private:
        static OpcodeDataType opcodeMask() {
            using Services::BitsetService;

            auto opcodeMask = static_cast<OpcodeDataType>(-1LL);

            if constexpr (decltype(sourceRegisterParameter)::hasValue()) {
                opcodeMask &= ~(sourceRegisterParameter.value.mask);
            }

            if constexpr (decltype(destinationRegisterParameter)::hasValue()) {
                opcodeMask &= ~(destinationRegisterParameter.value.mask);
            }

            if constexpr (decltype(dataParameter)::hasValue()) {
                opcodeMask &= ~(dataParameter.value.mask);
            }

            if constexpr (decltype(programAddressParameter)::hasValue()) {
                opcodeMask &= ~(programAddressParameter.value.mask);
            }

            if constexpr (decltype(programAddressOffsetParameter)::hasValue()) {
                opcodeMask &= ~(programAddressOffsetParameter.value.mask);
            }

            if constexpr (decltype(registerBitPositionParameter)::hasValue()) {
                opcodeMask &= ~(registerBitPositionParameter.value.mask);
            }

            if constexpr (decltype(statusRegisterBitPositionParameter)::hasValue()) {
                opcodeMask &= ~(statusRegisterBitPositionParameter.value.mask);
            }

            if constexpr (decltype(ioSpaceAddressParameter)::hasValue()) {
                opcodeMask &= ~(ioSpaceAddressParameter.value.mask);
            }

            if constexpr (decltype(dataSpaceAddressParameter)::hasValue()) {
                opcodeMask &= ~(dataSpaceAddressParameter.value.mask);
            }

            if constexpr (decltype(displacementParameter)::hasValue()) {
                opcodeMask &= ~(displacementParameter.value.mask);
            }

            return opcodeMask;
        }
    };
}
