@require(class_name, header_ext, include, enums, fields, global_vars, config_hash)

#ifndef @class_name.upper()_@header_ext.upper()
#define @class_name.upper()_@header_ext.upper()

#include <string>

@for line in include.split('/n'):
@line.strip()
@end

class @class_name
{
    static const long CONFIG_HASH = @str(config_hash)L;

    public:

@for enum_type,elems in enums.items():
    enum @enum_type
    {
@for elem in elems:
        @elem,
@end
    };
 
@end
    
    @class_name!s();
    
    bool set_value(const std::string& field_name, 
                   const std::string& field_value);

    bool parse_args(int& argc, char** argv);

    static bool load_file(const std::string& filename, @class_name& @class_name.lower(), 
                          bool& needs_resave);
    static bool save_file(const std::string& filename, 
                          const @class_name& @class_name.lower());

@for field in fields:

    @field['type'] @field['name']!s() { return _@field['name']; }
    void set_@field['name']!s(@field['type'] v) { _@field['name'] = v; }
@end

    private:
@for field in fields:

@for line in [line.strip() for line in field['comment'].split('\n') if len(line.strip()) > 0]:
    // @line
@end
    @field['type'] _@field['name'];
@end

};


@for global_var in global_vars:
extern @class_name @global_var;
@end
 
@for enum_type,elems in enums.items():
// String converter functions for @enum_type
template<> inline bool from_string(const string& s, @class_name::@enum_type& t)
{
@for elem in elems:
    if (s == "@elem") {t = @class_name::@elem; return true; }
@end
    
    return false;
}

template<> inline string to_string(const @class_name::@enum_type& t) {
    switch(t) {
@for elem in elems:
    case @class_name::@elem:
        return "@elem";
@end
    default:
        return "unknown";
    }
}

@end

                                
#endif

