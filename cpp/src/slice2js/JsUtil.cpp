//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "JsUtil.h"
#include <Slice/Util.h>
#include <IceUtil/StringUtil.h>

#include <algorithm>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#   include <direct.h>
#else
#   include <unistd.h>
#endif

// TODO: fix this warning!
#if defined(_MSC_VER)
#   pragma warning(disable:4456) // shadow
#   pragma warning(disable:4457) // shadow
#   pragma warning(disable:4459) // shadow
#elif defined(__clang__)
#   pragma clang diagnostic ignored "-Wshadow"
#elif defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wshadow"
#endif

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

string
Slice::relativePath(const string& p1, const string& p2)
{
    vector<string> tokens1;
    vector<string> tokens2;

    splitString(p1, "/\\", tokens1);
    splitString(p2, "/\\", tokens2);

    string f1 = tokens1.back();
    string f2 = tokens2.back();

    tokens1.pop_back();
    tokens2.pop_back();

    vector<string>::const_iterator i1 = tokens1.begin();
    vector<string>::const_iterator i2 = tokens2.begin();

    while(i1 != tokens1.end() && i2 != tokens2.end() && *i1 == *i2)
    {
        i1++;
        i2++;
    }

    //
    // Different volumes, relative path not possible.
    //
    if(i1 == tokens1.begin() && i2 == tokens2.begin())
    {
        return p1;
    }

    string newPath;
    if(i2 == tokens2.end())
    {
        newPath += "./";
        for (; i1 != tokens1.end(); ++i1)
        {
            newPath += *i1 + "/";
        }
    }
    else
    {
        for(vector<string>::difference_type i = tokens2.end() - i2; i > 0; i--)
        {
            newPath += "../";
        }

        for(; i1 != tokens1.end(); ++i1)
        {
            newPath += *i1 + "/";
        }
    }
    newPath += f1;

    return newPath;
}

static string
lookupKwd(const string& name)
{
    //
    // Keyword list. *Must* be kept in alphabetical order.
    //
    static const string keywordList[] =
    {
        "await", "break", "case", "catch", "class", "const", "continue", "debugger", "default", "delete", "do",
        "else", "enum", "export", "extends", "false", "finally", "for", "function", "if", "implements", "import",
        "in", "instanceof", "interface", "let", "new", "null", "package", "private", "protected", "public", "return",
        "static", "super", "switch", "this", "throw", "true", "try", "typeof", "var", "void", "while", "with",
        "yield"
    };
    bool found = binary_search(&keywordList[0],
                               &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
                               name,
                               Slice::CICompare());
    if(found)
    {
        return "_" + name;
    }

    return name;
}

static vector<string>
fixIds(const vector<string>& ids)
{
    vector<string> newIds;
    for(vector<string>::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        newIds.push_back(lookupKwd(*i));
    }
    return newIds;
}

string
Slice::JsGenerator::getDefinedIn(const ContainedPtr& p)
{
    static const string prefix = "js:defined-in:";
    string definedIn;
    if(findMetaData(prefix, p->getMetaData(), definedIn))
    {
        string base = p->definitionContext()->filename();
        string::size_type pos = base.find_last_of("/\\");
        assert(pos != string::npos);
        base = base.erase(pos);
        return fullPath(base + "/" + definedIn);
    }
    return "";
}

string
Slice::JsGenerator::getModuleMetadata(const TypePtr& type)
{
    static const char* builtinModuleTable[] =
    {
        "",           // byte
        "",           // bool
        "",           // short
        "",           // int
        "ice",        // long
        "",           // float
        "",           // double
        "",           // string
        "ice",        // Ice.Value
        "ice",        // Ice.ObjectPrx
        "ice"         // Ice.Object
    };

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if(builtin)
    {
        return builtinModuleTable[builtin->kind()];
    }

    return getModuleMetadata(dynamic_pointer_cast<Contained>(type));
}

string
Slice::JsGenerator::getModuleMetadata(const ContainedPtr& p)
{
    //
    // Check if the file contains the js:module file metadata.
    //
    DefinitionContextPtr dc = p->definitionContext();
    assert(dc);
    const string prefix = "js:module:";
    const string value = dc->findMetaData(prefix);
    return value.empty() ? value : value.substr(prefix.size());
}

bool
Slice::JsGenerator::isClassType(const TypePtr& type)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    return (builtin && (builtin->kind() == Builtin::KindObject || builtin->kind() == Builtin::KindValue)) ||
        dynamic_pointer_cast<ClassDecl>(type);
}

//
// If the passed name is a scoped name, return the identical scoped name,
// but with all components that are JS keywords replaced by
// their "_"-prefixed version; otherwise, if the passed name is
// not scoped, but a JS keyword, return the "_"-prefixed name.
//
string
Slice::JsGenerator::fixId(const string& name)
{
    if(name.empty())
    {
        return name;
    }
    if(name[0] != ':')
    {
        return lookupKwd(name);
    }

    const vector<string> ids = splitScopedName(name);
    const vector<string> newIds = fixIds(ids);

    stringstream result;
    for(vector<string>::const_iterator j = newIds.begin(); j != newIds.end(); ++j)
    {
        if(j != newIds.begin())
        {
            result << '.';
        }
        result << *j;
    }
    return result.str();
}

string
Slice::JsGenerator::fixId(const ContainedPtr& cont)
{
    return fixId(cont->name());
}

string
Slice::JsGenerator::importPrefix(const TypePtr& type,
                                 const ContainedPtr& toplevel,
                                 const vector<pair<string, string> >& imports)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if(builtin)
    {
        return typeToString(type, toplevel, imports, true);
    }
    else if(dynamic_pointer_cast<InterfaceDecl>(type))
    {
        InterfaceDeclPtr proxy = dynamic_pointer_cast<InterfaceDecl>(type);
        return importPrefix(dynamic_pointer_cast<Contained>(proxy), toplevel, imports);
    }
    else if(dynamic_pointer_cast<Contained>(type))
    {
        ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
        if(cl)
        {
            if(cl->definition())
            {
                return importPrefix(dynamic_pointer_cast<Contained>(cl->definition()), toplevel, imports);
            }
            else
            {
                return importPrefix(dynamic_pointer_cast<Contained>(cl), toplevel, imports, getDefinedIn(cl));
            }
        }
        else
        {
            return importPrefix(dynamic_pointer_cast<Contained>(type), toplevel, imports);
        }
    }
    return "";
}

string
Slice::JsGenerator::importPrefix(const ContainedPtr& contained,
                                 const ContainedPtr& toplevel,
                                 const vector<pair<string, string> >& imports,
                                 const string& definedIn)
{
    string m1 = getModuleMetadata(contained);
    string m2 = getModuleMetadata(toplevel);

    string p;

    if(m1.empty())
    {
        string p1 = definedIn.empty() ? contained->definitionContext()->filename() : definedIn;
        string p2 = toplevel->definitionContext()->filename();

        p = relativePath(p1, p2);

        string::size_type pos = p.rfind('.');
        if (pos != string::npos)
        {
            p.erase(pos);
        }
    }
    else if(m1 == "ice" && m1 != m2)
    {
        return "iceNS0.";
    }
    else if(m1 != m2)
    {
        p = m1;
    }

    if(!p.empty())
    {
        for(vector<pair<string, string> >::const_iterator i = imports.begin(); i != imports.end(); ++i)
        {
            if(i->first == p)
            {
                return i->second + ".";
            }
        }
    }

    return "";
}

bool
Slice::JsGenerator::findMetaData(const string& prefix, const StringList& metaData, string& value)
{
    for(StringList::const_iterator i = metaData.begin(); i != metaData.end(); i++)
    {
        string s = *i;
        if(s.find(prefix) == 0)
        {
            value = s.substr(prefix.size());
            return true;
        }
    }
    return false;
}

string
Slice::JsGenerator::importPrefix(const string& type, const ContainedPtr& toplevel)
{
    const string module = getModuleMetadata(toplevel);
    return (type.find("Ice.") == 0 && module != "ice") ? "iceNS0." : "";
}

string
Slice::JsGenerator::getUnqualified(const string& type, const string& scope, const string& importPrefix)
{
    if(importPrefix.empty())
    {
        const string localScope = getLocalScope(scope) + ".";
        if(type.find(localScope) == 0)
        {
            string t = type.substr(localScope.size());
            if(t.find(".") == string::npos)
            {
                return t;
            }
        }
    }
    return type;
}

string
Slice::JsGenerator::typeToString(const TypePtr& type,
                                 const ContainedPtr& toplevel,
                                 const vector<pair<string, string> >& imports,
                                 bool typescript,
                                 bool definition)
{
    if(!type)
    {
        return "void";
    }

    static const char* typeScriptBuiltinTable[] =
    {
        "number",           // byte
        "boolean",          // bool
        "number",           // short
        "number",           // int
        "Ice.Long",         // long
        "number",           // float
        "number",           // double
        "string",
        "Ice.Object",
        "Ice.ObjectPrx",
        "Ice.Value"
    };

    static const char* javaScriptBuiltinTable[] =
    {
        "Number",           // byte
        "Boolean",          // bool
        "Number",           // short
        "Number",           // int
        "Ice.Long",         // long
        "Number",           // float
        "Number",           // double
        "String",
        "Ice.Value",
        "Ice.ObjectPrx",
        "Ice.Value"
    };

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if(builtin)
    {
        if(typescript)
        {
            int kind = (builtin->kind() == Builtin::KindObject) ? Builtin::KindValue : builtin->kind();
            ostringstream os;
            if(getModuleMetadata(type) == "ice" && getModuleMetadata(toplevel) != "ice")
            {
                os << "iceNS0.";
            }
            os << getUnqualified(typeScriptBuiltinTable[kind], toplevel->scope(), "iceNS0.");
            return os.str();
        }
        else
        {
            return javaScriptBuiltinTable[builtin->kind()];
        }
    }

    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    if(cl)
    {
        string prefix;
        ostringstream os;
        if(typescript)
        {
            if(cl->definition())
            {
                prefix = importPrefix(dynamic_pointer_cast<Contained>(cl->definition()), toplevel, imports);
            }
            else
            {
                prefix = importPrefix(dynamic_pointer_cast<Contained>(cl), toplevel, imports, getDefinedIn(cl));
            }
        }
        os << prefix;
        if(!prefix.empty() && typescript)
        {
            os << getUnqualified(fixId(cl->scoped()), toplevel->scope(), prefix);
        }
        else
        {
            os << fixId(cl->scoped());
        }
        return os.str();
    }

    InterfaceDeclPtr proxy = dynamic_pointer_cast<InterfaceDecl>(type);
    if(proxy)
    {
        ostringstream os;

        string prefix;
        if(typescript)
        {
            prefix = importPrefix(dynamic_pointer_cast<Contained>(proxy), toplevel, imports);
            os << prefix;
        }

        if(prefix.empty() && typescript)
        {
            os << getUnqualified(fixId(proxy->scoped() + "Prx"), toplevel->scope(), prefix);
        }
        else
        {
            os << fixId(proxy->scoped() + "Prx");
        }

        return os.str();
    }

    if(!typescript || definition)
    {
        SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
        if (seq)
        {
            BuiltinPtr b = dynamic_pointer_cast<Builtin>(seq->type());
            if (b && b->kind() == Builtin::KindByte)
            {
                return "Uint8Array";
            }
            else
            {
                return typeToString(seq->type(), toplevel, imports, typescript) + "[]";
            }
        }

        DictionaryPtr d = dynamic_pointer_cast<Dictionary>(type);
        if(d)
        {
            const TypePtr keyType = d->keyType();
            BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(keyType);
            ostringstream os;
            if ((builtin && builtin->kind() == Builtin::KindLong) || dynamic_pointer_cast<Struct>(keyType))
            {
                const string prefix = importPrefix("Ice.HashMap", toplevel);
                os << prefix << getUnqualified("Ice.HashMap", toplevel->scope(), prefix);
            }
            else
            {
                os << "Map";
            }

            if (typescript)
            {
                os << "<"
                    << typeToString(keyType, toplevel, imports, true) << ", "
                    << typeToString(d->valueType(), toplevel, imports, true) << ">";
            }
            return os.str();
        }
    }

    ContainedPtr contained = dynamic_pointer_cast<Contained>(type);
    if(contained)
    {
        ostringstream os;
        string prefix;
        if(typescript)
        {
            prefix = importPrefix(contained, toplevel, imports);
            os << prefix;
        }

        if(prefix.empty() && typescript)
        {
            os << getUnqualified(fixId(contained->scoped()), toplevel->scope(), prefix);
        }
        else
        {
            os << fixId(contained->scoped());
        }
        return os.str();
    }

    return "???";
}

string
Slice::JsGenerator::typeToString(const TypePtr& type,
                                 const ContainedPtr& toplevel,
                                 const std::vector<std::pair<std::string, std::string> >& imports,
                                 bool typeScript,
                                 bool definition,
                                 bool usealias)
{
    string t = typeToString(type, toplevel, imports, typeScript, definition);
    if(usealias)
    {
        string m1 = getModuleMetadata(type);
        string m2 = getModuleMetadata(toplevel);
        if (!m1.empty() && m1 == m2)
        {
            // we are using the same module
            return t;
        }
        string p = importPrefix(type, toplevel, imports);

        //
        // When using an import prefix we don't need an alias, prefixes use iceNSXX that is reserved
        // name prefix
        //
        string::size_type i = t.find(".");
        if(p.empty() && i != string::npos)
        {
            const string scoped = fixId(toplevel->scoped()) + ".";
            if(scoped.find("." + t.substr(0, i + 1)) != string::npos)
            {
                replace(t.begin(), t.end(), '.', '_');
                t = "iceA_" + t;
            }
        }
    }
    return t;
}

string
Slice::JsGenerator::getLocalScope(const string& scope, const string& separator)
{
    assert(!scope.empty());

    //
    // Remove trailing "::" if present.
    //
    string fixedScope;
    if(scope[scope.size() - 1] == ':')
    {
        assert(scope[scope.size() - 2] == ':');
        fixedScope = scope.substr(0, scope.size() - 2);
    }
    else
    {
        fixedScope = scope;
    }

    if(fixedScope.empty())
    {
        return "";
    }
    const vector<string> ids = fixIds(splitScopedName(fixedScope));

    //
    // Return local scope for "::A::B::C" as A.B.C
    //
    stringstream result;
    for(vector<string>::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        if(i != ids.begin())
        {
            result << separator;
        }
        result << *i;
    }
    return result.str();
}

void
Slice::JsGenerator::writeMarshalUnmarshalCode(Output &out,
                                              const TypePtr& type,
                                              const string& param,
                                              bool marshal)
{
    string stream = marshal ? "ostr" : "istr";

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindByte:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeByte(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readByte()" << ';';
                }
                return;
            }
            case Builtin::KindBool:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeBool(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readBool()" << ';';
                }
                return;
            }
            case Builtin::KindShort:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeShort(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readShort()" << ';';
                }
                return;
            }
            case Builtin::KindInt:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeInt(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readInt()" << ';';
                }
                return;
            }
            case Builtin::KindLong:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeLong(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readLong()" << ';';
                }
                return;
            }
            case Builtin::KindFloat:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeFloat(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readFloat()" << ';';
                }
                return;
            }
            case Builtin::KindDouble:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeDouble(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readDouble()" << ';';
                }
                return;
            }
            case Builtin::KindString:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeString(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readString()" << ';';
                }
                return;
            }
            case Builtin::KindObject:
            case Builtin::KindValue:
            {
                // Handle by isClassType below.
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if(marshal)
                {
                    out << nl << stream << ".writeProxy(" << param << ");";
                }
                else
                {
                    out << nl << param << " = " << stream << ".readProxy();";
                }
                return;
            }
        }
    }

    if(dynamic_pointer_cast<Enum>(type))
    {
        if(marshal)
        {
            out << nl << typeToString(type) << "._write(" << stream << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = " << typeToString(type) << "._read(" << stream << ");";
        }
        return;
    }

    if(dynamic_pointer_cast<InterfaceDecl>(type) || dynamic_pointer_cast<Struct>(type))
    {
        if(marshal)
        {
            out << nl << typeToString(type) << ".write(" << stream << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = " << typeToString(type) << ".read(" << stream << ", " << param << ");";
        }
        return;
    }

    if(isClassType(type))
    {
        if(marshal)
        {
            out << nl << stream << ".writeValue(" << param << ");";
        }
        else
        {
            out << nl << stream << ".readValue(obj => " << param << " = obj, ";
            out << typeToString(type) << ");";
        }
        return;
    }

    if(dynamic_pointer_cast<Sequence>(type) || dynamic_pointer_cast<Dictionary>(type))
    {
        if(marshal)
        {
            out << nl << getHelper(type) <<".write(" << stream << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = " << getHelper(type) << ".read(" << stream << ");";
        }
        return;
    }

    assert(false);
}

void
Slice::JsGenerator::writeOptionalMarshalUnmarshalCode(Output &out,
                                                      const TypePtr& type,
                                                      const string& param,
                                                      int tag,
                                                      bool marshal)
{
    string stream = marshal ? "ostr" : "istr";

    if(isClassType(type))
    {
        if(marshal)
        {
            out << nl << stream << ".writeOptionalValue(" << tag << ", " << param << ");";
        }
        else
        {
            out << nl << stream << ".readOptionalValue(" << tag << ", obj => " << param << " = obj, "
                << typeToString(type) << ");";
        }
        return;
    }

    if(dynamic_pointer_cast<Enum>(type))
    {
        if(marshal)
        {
            out << nl << typeToString(type) <<"._writeOpt(" << stream << ", " << tag << ", " << param << ");";
        }
        else
        {
            out << nl << param << " = " << typeToString(type) << "._readOpt(" << stream << ", " << tag << ");";
        }
        return;
    }

    if(marshal)
    {
        out << nl << getHelper(type) <<".writeOptional(" << stream << ", " << tag << ", " << param << ");";
    }
    else
    {
        out << nl << param << " = " << getHelper(type) << ".readOptional(" << stream << ", " << tag << ");";
    }
}

std::string
Slice::JsGenerator::getHelper(const TypePtr& type)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if(builtin)
    {
        switch(builtin->kind())
        {
            case Builtin::KindByte:
            {
                return "Ice.ByteHelper";
            }
            case Builtin::KindBool:
            {
                return "Ice.BoolHelper";
            }
            case Builtin::KindShort:
            {
                return "Ice.ShortHelper";
            }
            case Builtin::KindInt:
            {
                return "Ice.IntHelper";
            }
            case Builtin::KindLong:
            {
                return "Ice.LongHelper";
            }
            case Builtin::KindFloat:
            {
                return "Ice.FloatHelper";
            }
            case Builtin::KindDouble:
            {
                return "Ice.DoubleHelper";
            }
            case Builtin::KindString:
            {
                return "Ice.StringHelper";
            }
            case Builtin::KindObject:
            case Builtin::KindValue:
            {
                return "Ice.ObjectHelper";
            }
            case Builtin::KindObjectProxy:
            {
                return "Ice.ObjectPrx";
            }
        }
    }

    if(dynamic_pointer_cast<Enum>(type))
    {
        return typeToString(type) + "._helper";
    }

    if(dynamic_pointer_cast<Struct>(type))
    {
        return typeToString(type);
    }

    InterfaceDeclPtr prx = dynamic_pointer_cast<InterfaceDecl>(type);
    if(prx)
    {
        return typeToString(type);
    }

    if(dynamic_pointer_cast<Sequence>(type) || dynamic_pointer_cast<Dictionary>(type))
    {
        stringstream s;
        s << getLocalScope(dynamic_pointer_cast<Contained>(type)->scoped()) << "Helper";
        return s.str();
    }

    if(dynamic_pointer_cast<ClassDecl>(type))
    {
        return "Ice.ObjectHelper";
    }

    assert(false);
    return "???";
}
