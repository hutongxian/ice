//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Slice/Util.h>
#include "PHPUtil.h"
#include <functional>
#include <algorithm>
#include <vector>

using namespace std;
using namespace Slice;

static string
lowerCase(const string& s)
{
    string result(s);
    transform(result.begin(), result.end(), result.begin(),
              [](char c){ return static_cast<char>(::tolower(static_cast<unsigned char>(c))); });
    return result;
}

static string
lookupKwd(const string& name)
{
    string lower = lowerCase(name); // PHP is case insensitive.

    //
    // Keyword list. *Must* be kept in alphabetical order.
    //
    static const string keywordList[] =
    {
        "_halt_compiler", "abstract", "and", "array", "as", "break", "callable", "case", "catch", "class", "clone",
        "const", "continue", "declare", "default", "die", "do", "echo", "else", "elseif", "empty", "enddeclare",
        "endfor", "endforeach", "endif", "endswitch", "endwhile", "eval", "exit", "extends", "final", "finally",
        "for", "foreach", "function", "global", "goto", "if", "implements", "include", "include_once", "instanceof",
        "insteadof", "interface", "isset", "list", "namespace", "new", "or", "print", "private", "protected", "public",
        "require", "require_once", "return", "static", "switch", "this", "throw", "trait", "try", "unset", "use", "var",
        "while", "xor", "yield"
    };
    bool found =  binary_search(&keywordList[0],
                                &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
                                lower);
    return found ? "_" + name : name;
}

string
Slice::PHP::scopedToName(const string& scoped, bool ns)
{
    string result;
    if(ns)
    {
        result = fixIdent(scoped);
        if(result.find("::") == 0)
        {
            result.replace(0, 2, "\\");
        }

        string::size_type pos;
        while((pos = result.find("::")) != string::npos)
        {
            result.replace(pos, 2, "\\");
        }
    }
    else
    {
        string str = scoped;
        if(str.find("::") == 0)
        {
            str.erase(0, 2);
        }

        string::size_type pos;
        while((pos = str.find("::")) != string::npos)
        {
            str.replace(pos, 2, "_");
        }

        result = fixIdent(str);
    }

    return result;
}

string
Slice::PHP::fixIdent(const string& ident)
{
    if(ident[0] != ':')
    {
        return lookupKwd(ident);
    }
    vector<string> ids = splitScopedName(ident);
    transform(ids.begin(), ids.end(), ids.begin(), [](const string& id) -> string { return lookupKwd(id); });
    stringstream result;
    for(vector<string>::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        result << "::" + *i;
    }
    return result.str();
}

string
Slice::PHP::escapeName(const string& name)
{
    string result = name;

    string::size_type pos = 0;
    while((pos = result.find("\\", pos)) != string::npos)
    {
        result.insert(pos, "\\");
        pos += 2;
    }

    return result;
}
