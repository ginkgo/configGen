@require(class_name, header_ext, include, enums, fields, global_vars, config_hash)

#include "@class_name!s.@header_ext"

#include <iostream>
#include <fstream>
#include <boost/regex.hpp>

@for global_var in global_vars:
@class_name @global_var;
@end

@class_name::@class_name!s() 
{
@for no,field in enumerate(fields):
    set_value("@field['name']", "@field['default']");
@end
}

bool @class_name::set_value(const std::string& field_name,
                            const std::string& field_value)
{    
@for field in fields:
    if (field_name == "@field['name']") return from_string(field_value, _@field['name']);
@end

    return false;
}

bool @class_name::parse_args(int& argc, char** argv)
{
    const boost::regex pattern("--(\\w+)=(.+)$");
    boost::match_results<std::string::const_iterator> match;
    
    int other = 0;

    for (int i = 0; i < argc; ++i) {
		string arg(argv[i]);
        if (boost::regex_match(arg, match, pattern)) {
            bool success = set_value(match[1], match[2]);

            if (!success) {
                cerr << "Failed to set " << match[1] 
                     << " to " << match[2] << endl;
                return false;
            }
        } else {
            argv[other] = argv[i];
            other++;
        }
    }

    argc = other;

    return true;
}

bool @class_name::save_file(const std::string& filename,
                            const @class_name& config)
{
    std::ofstream os(filename.c_str());

    if (!os) {
        std::cerr << "Could not open file " << filename << " for writing." << std::endl;
        return false;
    }

@for field in fields:
@for line in [line.strip() for line in field['comment'].split('\n') if len(line.strip()) > 0]:
    os << "// @line" << std::endl;
@end
    os << "@field['name'] = " << to_string(config._@field['name']) << std::endl;
    os << std::endl;

@end

    os << std::endl << std::endl;
    os << "#hash:" << CONFIG_HASH << std::endl;

    return true;
}

bool @class_name::load_file(const std::string& filename,
                            @class_name& config, bool& needs_resave)
{
    needs_resave = true;

    const int MAX_LENGTH = 1000;
    char line[MAX_LENGTH];

    const boost::regex pattern("\\s*(\\w+)\\s+=\\s+(.+)\\s*$");
    const boost::regex version_pattern("#hash:(-?[0-9]+)$");
    boost::match_results<std::string::const_iterator> match;

    std::ifstream is(filename.c_str());

    if (!is) {
        return false;
    }

    while(is.getline(line, MAX_LENGTH)) {
        std::string strline(line);
        if (boost::regex_match(strline, match, pattern)) {
            bool success = config.set_value(match[1], match[2]);

            if (!success) {
                return false;
            }
        } else if (boost::regex_match(strline, match, version_pattern)) {
            long config_hash = 0;
            from_string<long>(match[1], config_hash);

            if (config_hash == CONFIG_HASH) {
                needs_resave = false;
            } else {
                needs_resave = true;
            }
        }
    }

    return true;
}
