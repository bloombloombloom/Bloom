## Target description files

A target description file (TDF) is an XML file that describes a particular target. All targets that are supported by
Bloom will have an associated TDF in Bloom's codebase. They can be found in `src/Targets/TargetDescriptionFiles`.

The data held by TDFs is fundamental to Bloom's operation. The following is an incomplete list of some of the
data extracted from TDFs:

- Target signatures
- Parameters for debug interfaces
- Memory address space descriptions (including sections, offsets and sizes)
- Register descriptions (including offsets and sizes)
- Package variant descriptions
- Package pinouts

Given that Bloom currently only supports AVR8 targets, we only possess TDFs for AVR8 targets.

TDFs are distributed with Bloom. They are copied to the distribution directory
(`build/resources/TargetDescriptionFiles/`) at build time and included in the Debian installation package. Upon copying
the TDFs, we also generate a JSON mapping of AVR8 target signatures to TDF file paths. This mapping is also
distributed with Bloom and can be found in the `build/resources/TargetDescriptionFiles/AVR` directory, with the
name `Mapping.json`. The mapping is used by Bloom, at runtime, to resolve the appropriate TDF from an AVR8 target
signature. The TDF file paths in the mapping are relative to Bloom's executable path.
See `Avr8::loadTargetDescriptionFile()` for more. See `build/scripts/Avr8TargetDescriptionFiles.php` for the script
that performs the copying and generation of the JSON mapping.

### TDF format

AVR8 TDFs were initially taken from the Microchip Packs Repository (https://packs.download.microchip.com/). Some
changes to the format have been made to better suit Bloom's requirements.

TODO: Explain the TDF format. For now, developers can gain an understanding of the format by viewing a TDF.

### TDF parsing

The `src/Targets/TargetDescription/` directory contains the necessary code and data structures to parse
and represent generic TDFs, with the `Bloom::Targets::TargetDescription::TargetDescriptionFile` class being the entry
point.

#### Extending the TargetDescriptionFile class for TDFs with formats that are specific to certain target families and/or architectures

AVR8 TDFs describe certain constructs in a way that **may** not be employed by other target families. For example,
AVR8 PORT registers are described in a `<module>` node of the TDF. This method is currently considered to be specific
to AVR8 TDFs. For that reason, we extend the `Bloom::Targets::TargetDescription::TargetDescriptionFile` class to better
represent TDFs for particular target families or architectures. For AVR8 targets, we use the
`Bloom::Targets::Microchip::Avr::Avr8Bit::TargetDescription::TargetDescriptionFile` class to represent AVR8 TDFs.

It would be good to keep in mind that as and when support for other target families and architectures is implemented,
we may use the constructs that were initially specific to AVR8 TDFs, in other TDFs. In this case, those constructs will
likely be moved into the generic `Bloom::Targets::TargetDescription::TargetDescriptionFile` class.

### TDF validation

In order to ensure that every TDF in Bloom's codebase is in the correct format, and meets the minimum requirements to be
of use to Bloom, we perform validation on each TDF at build time. If even one TDF fails validation, the build is aborted.
This validation takes place in the `build/scripts/Avr8TargetDescriptionFiles.php` script.

There is also a separate script for AVR8 TDF validation, for developers to invoke manually, outside of builds,
when needed: `build/scripts/ValidateAvr8TargetDescriptionFiles.php`
