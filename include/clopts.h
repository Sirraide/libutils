#ifndef UTILS_CLOPTS_H
#define UTILS_CLOPTS_H

#include "./utils.h"

#include <cctype>
#include <string_view>
#include <variant>
#include <vector>

LIBUTILS_NAMESPACE_BEGIN

/**
 * Struct that stores and contains functions for parsing command line arguments
 */
struct Clopts {
    /**  Command line option value */
    using Value = std::variant<std::string, U64, I64, F64, bool>;

    /**  The type of the argument that a command line option takes */
    enum struct Type {
        /**  No argument. I.e. this option is a flag */
        Void,

        /**  This option takes an std::string */
        String,

        /** This option consumes the remaining arguments as a string */
        Rest,

        /**  This option takes a 64-bit unsigned integer */
        U64,

        /**  This option take a 64-bit signed integer */
        I64,

        /**  This option takes a 64-bit floating-point number */
        F64,

        /**  This option takes ‘true’ or ‘false’ */
        Bool
    };

    /**  Convert a Type to the type represented by it */
    template <Type T>
    using TypeEnumToDataType = type_if_t<T == Type::Void, bool,
        type_if_t<T == Type::String, std::string,
            type_if_t<T == Type::U64, U64,
                type_if_t<T == Type::I64, I64,
                    type_if_t<T == Type::F64, F64, bool>>>>>;

    /**  Global option id */
    static U64 o_id;

    /**
     * Instances of this class represent a single command line option
     */
    class Option {
        /** The id of this option */
        U64 id = o_id++;

        /**  The Type of this option */
        Type type;

        /**  Whether this option has already been found */
        bool found = false;

        /**  The argument of this option */
        Value value{};

    public:
        /**
         * The values above might confuse the user, hence why they are private.
         * We still need to access them though.
         */
        friend struct Clopts;

        /**  The name of this option */
        std::string name;

        /**  The description to be displayed in the help information */
        std::string description;

        /**  Whether this option must appear on the command line */
        bool required;

        /**  Anonymous options gobble up any command line arguments that do not correspond to other options */
        bool anonymous;

        /**  Whether this option may occur multiple times */
        bool allow_duplicates;

        /**
         * Create a new command line option
         * <p>
         * Constructing an option does nothing until it is passed to the constructor of Clopts
         * @param name The name of the option
         * @param description The description to be included in the help information; defaults to ""
         * @param type The type of the option; defaults to String
         * @param required Whether the option is required; defaults to false
         * @param anonymous Whether the option is anonymous; defaults to false
         * @param allow_duplicates Whether this option may occur multiple times; defaults to false
         */
        Option(std::string name, std::string descr = "", Type type = Type::String, bool required = false,
            bool anonymous = false, bool allow_duplicates = false);

        /**
         * Compare the name of this option with a string
         * <p>
         * Used to find options by name
         * @tparam TString The string type to use
         * @param _name The string to compare with
         * @return true if the name of this option is equal to `_name' and false otherwise
         */
        template <typename TString>
        auto operator==(const TString& _name) const { return name == _name; }

        /**
         * Return the string representation of this option’s type
         */
        [[nodiscard]] std::string TypeAsStr() const;

        inline Type               OptType() const { return type; }
        inline bool               Found() const { return found; }
        inline const std::string& AsRest() const { return std::get<std::string>(value); }
        inline const std::string& AsString() const { return std::get<std::string>(value); }
        inline U64                AsU64() const { return std::get<U64>(value); }
        inline I64                AsI64() const { return std::get<I64>(value); }
        inline F64                AsF64() const { return std::get<F64>(value); }
        inline bool               AsBool() const { return std::get<bool>(value); }
    };

private:
    /** We need argv[0] in Usage() */
    std::string program_name = "(null)";

    /** The named options */
    std::vector<Option> options;

    /** The anonymous options */
    std::vector<Option> anonymous;

    /** The number of command line arguments required */
    U64 opts_required{};

    /** Whether an unknown option does not constitute an error */
    bool allow_unknown = false;

    /** Whether we should print usage information when encountering -h or --help */
    bool have_help_flag = true;

    /**
     * Construct the options to be used by the parser
     * @param lst The list of options
     */
    void ConstructOptions(const std::initializer_list<Option>& lst);

    /**
     * Parse the option value
     * @param type The type of the value
     * @param text The text to be parsed as the option value
     * @return The parsed option value
     */
    auto ParseValue(Type type, const std::string_view& text) -> Value;

    /**
     * Consume the rest of the command line
     * @param opt The option in which to save the rest of the command line
     * @param text The contents of the first argument of the rest
     * @param argc The number of arguments
     * @param argv The argument vector
     * @param i The current position in the command line
     */
    void ConsumeRest(Option& opt, const std::string_view& text, int argc, char** argv, int i);

    /**
     * Verify that no two options have the same name
     * @param opts The list of options
     */
    void VerifyDistinct(const std::vector<Option>& opts);

public:
    /**
     * Construct a command-line-argument parser from a list of options
     * @param lst The list of options
     */
    Clopts(std::initializer_list<Option> lst);

    /**
     * Toggle whether unkown options should constitute an error
     * @param allow Whether unkown options should be allowed
     */
    void AllowUnknown(bool allow);

    /**
     * Toggle whether implicit help flags should be enabled
     * <p>
     * If the help flag is enabled, '-h' and '--help' will cause
     * the parser to print the Usage() information and immediately exit.
     * <p>
     * This takes precedence over any '-h' or '--help' flags provided
     * by the user.
     * @param enable Whether to enable implicit help flags
     */
    void EnableHelpFlag(bool enable);

    /**
     * Parse the command line
     * @param argc The number of arguments
     * @param argv The arguments
     */
    void Parse(int argc, char** argv);

    /**
     * Generate the usage information for this parser
     * @return A string containing the usage information.
     * The string returned does not contain a trailing newline.
     */
    auto Usage() -> std::string;

    /**
     * Get an option
     * @param name The name of the option
     * @return The option, if found
     */
    auto operator[](const std::string& name) const -> const Option&;
};

/** For convenience */
using CT = Clopts::Type;

LIBUTILS_NAMESPACE_END

#endif // UTILS_CLOPTS_H
