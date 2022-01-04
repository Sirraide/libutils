#include "../include/clopts.h"

#define CLOPTS_ERROR(format, ...) Die(format "\033[m\n%s" __VA_OPT__(, ) __VA_ARGS__, Usage().data())

LIBUTILS_NAMESPACE_BEGIN

U64 Clopts::o_id{};

Clopts::Option::Option(std::string _name, std::string _descr, Type _type, bool _required, bool _anonymous, bool _allow_duplicates)
    : type(_type), name(std::move(_name)), description(std::move(_descr)), required(_required),
      anonymous(_anonymous), allow_duplicates(_allow_duplicates) {}

[[nodiscard]] std::string Clopts::Option::TypeAsStr() const {
    using enum Type;
    switch (type) {
        case Void: return "";
        case String: return "string";
        case U64:
        case I64:
        case F64: return "number";
        case Bool: return "true|false";
    }
}

Clopts::Clopts(std::initializer_list<Option> lst) {
    ConstructOptions(lst);
    VerifyDistinct(options);
    VerifyDistinct(anonymous);
}

void Clopts::ConstructOptions(const std::initializer_list<Option>& lst) {
    for (auto& option : lst) {
        /// Make sure the option has a name
        if (option.name.empty())
            Die("Clopts: Options cannot have an empty name. If you want to use "
                "an anonymous option, provide a dummy name and set the "
                "`anonymous' flag in the constructor to true.");

        /// If this is an anonymous option, add it to a separate vector.
        /// Anonymous options cannot have arguments; if the option is required,
        /// increment `opts_required' by 1 only.
        if (option.anonymous) {
            if (option.type == Type::Void) Die("Clopts: An anonymous option cannot be of type Void");
            anonymous.push_back(option);
            if (option.required) opts_required++;

        }

        /// Otherwise, just append it to the options vector
        else {
            options.push_back(option);
            if (option.required) opts_required += 1 + (option.type != Type::Void);
        }
    }
}

void Clopts::VerifyDistinct(const std::vector<Option>& opts) {
    /// Make sure no two options have the same name
    for (auto& opt1 : opts)
        for (auto& opt2 : opts)
            if (opt1.name == opt2.name
                && opt1.id != opt2.id)
                Die("Clopts: Cannot have two options with the same name: %s%s%s",
                    opt1.anonymous ? "<" : "", opt1.name.data(), opt1.anonymous ? ">" : "");
}

std::string Clopts::Usage() {
    std::string usage = "Usage: " + program_name;
    for (const auto& opt : anonymous) usage += " <" + opt.name + ">";
    if (!options.empty() || have_help_flag) {
        usage += " [options]\n\nOptions:\n";
        if (have_help_flag) usage += "    -h, --help\n";
        for (const auto& opt : options) usage += "    " + opt.name + " <" + opt.TypeAsStr() + ">\n";
    } else return usage;

    /// Remove the final newline
    return usage.substr(0, usage.size() - 1);
}

void Clopts::Parse(int argc, char** argv) {
    program_name = Trim(std::string{argv[0]});
    for (int i = 1; i < argc; i++) {
        /// Empty strings are allowed as arguments, but not as options
        if (argv[i][0] == 0) {
            if (!allow_unknown) CLOPTS_ERROR("Unexpected empty argument: %s", argv[i]);
            continue;
        }

        std::string_view option_raw(Trim(std::string_view(argv[i])));
        std::string_view option(option_raw);

        /// Print the help information if the help flag is enabled
        if (have_help_flag) {
            if (option == "-h" || option == "--help") {
                std::cout << Usage() << "\n";
                exit(0);
            }
        }

        /// Search for an option with that name.
        /// The option may be an option that accepts a value
        /// in which case it might be combined with that value
        /// using an '=' sign.
        U64  pos    = option_raw.find('=');
        bool has_eq = pos != std::string_view::npos;
        if (has_eq) option = option_raw.substr(0, pos);
        auto opt = std::find(options.begin(), options.end(), option);

        /// The option was found
        if (opt != options.end()) {
            /// Make sure that either this option hasn't been encountered yes,
            /// or that it allows duplicate options. If this option does not
            /// accept a value, continue.
            if (opt->found == !opt->allow_duplicates) CLOPTS_ERROR("Duplicate option: %s", argv[i]);
            opt->found = true;
            if (opt->type == Type::Void) continue;

            /// Otherwise, we need to parse the argument.
            if (has_eq) opt->value = ParseValue(opt->type, option_raw.substr(pos + 1));
            else {
                /// If no equals sign was found, make sure we have enough arguments
                if (++i == argc) CLOPTS_ERROR("Missing argument for option %*s", (int) option.size(), option.data());
                opt->value = ParseValue(opt->type, argv[i]);
            }
        }

        /// The option was not found, it might be an anonymous option
        else {
            /// Provide a different error message if there are no anonymous options
            if (anonymous.empty()) {
                if (!allow_unknown) CLOPTS_ERROR("Unrecognised option: %s", argv[i]);
                else continue;
            }

            /// Find the first anonymous option that doesn't have a value yet
            opt            = anonymous.begin();
            const auto end = anonymous.end();
            for (; opt != end; ++opt)
                if (!opt->found) break;

            /// For now, error if all anonymous slots have already been filled
            if (opt == end) {
                if (!allow_unknown) CLOPTS_ERROR("Unrecognised option: %s", argv[i]);
                else continue;
            }

            /// Otherwise, parse the value
            opt->found = true;
            opt->value = ParseValue(opt->type, argv[i]);
        }
    }

    /// After we're done parsing, make sure all required options have been found
    for (const auto& opt : options)
        if (opt.required && !opt.found) CLOPTS_ERROR("Option '%s' is required.", opt.name.data());
    for (const auto& opt : anonymous)
        if (opt.required && !opt.found) CLOPTS_ERROR("Option <%s> is required.", opt.name.data());
}

Clopts::Value Clopts::ParseValue(Type type, const std::string_view& text) {
    using enum Type;
    switch (type) {
        case String: return std::string(text);
        case Bool: {
            if (ToLower(std::string(text)) == "true") return true;
            if (ToLower(std::string(text)) == "false") return false;
            CLOPTS_ERROR("Invalid value. Expected ‘true’ or ‘false’, got: %*s", (int) text.size(), text.data());
        }
        case U64:
        case I64:
        case F64:
            Die("Number parsing is not supported yet");
        case Void: [[fallthrough]];
        default: Die("ParseValue(): Unreachable");
    }
}

void Clopts::AllowUnknown(bool allow) {
    allow_unknown = allow;
}

void Clopts::EnableHelpFlag(bool enable) {
    have_help_flag = enable;
}

LIBUTILS_NAMESPACE_END
