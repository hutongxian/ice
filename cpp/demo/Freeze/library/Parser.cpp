// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Parser.h>

#ifdef HAVE_READLINE
#   include <readline/readline.h>
#   include <readline/history.h>
#endif

using namespace std;
using namespace Ice;

extern FILE* yyin;

Parser* parser;

void
Parser::usage()
{
    cout <<
        "help                    Print this message.\n"
        "exit, quit              Exit this program.\n"
        "add isbn title authors  Create new book.\n"
        "isbn NUMBER             Find the book with given ISBN number.\n"
        "authors NAME            Find all books by the given authors.\n"
        "next                    Set the current book to the next one that was found.\n"
        "current                 Display the current book.\n"
        "rent NAME               Rent the current book for customer NAME.\n"
        "return                  Return the currently rented book.\n"
        "remove                  Permanently remove the current book from the library.\n"
        "size SIZE               Set the evictor size for books to SIZE.\n"
        "shutdown                Shut the library server down.\n";
}

ParserPtr
Parser::createParser(const CommunicatorPtr& communicator, const LibraryPrx& library)
{
    return new Parser(communicator, library);
}

void
Parser::addBook(const list<string>& _args)
{
    if (_args.size() != 3)
    {
	error("`add' requires at exactly three arguments (type `help' for more info)");
	return;
    }

    try
    {
	list<string> args = _args;

	BookDescription desc;
	desc.isbn = args.front();
	args.pop_front();
	desc.title = args.front();
	args.pop_front();
	desc.authors = args.front();

	BookPrx book = _library->createBook(desc);
	cout << "added new book with isbn " << desc.isbn << endl;
    }
    catch(const DatabaseException& ex)
    {
	error(ex.message);
    }
    catch(const BookExistsException&)
    {
	error("the book already exists.");
    }
    catch(const Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::findIsbn(const list<string>& args)
{
    if (args.size() != 1)
    {
	error("`isbn' requires exactly one argument (type `help' for more info)");
	return;
    }

    try
    {
	_foundBooks.clear();
	_current = _foundBooks.begin();

	BookPrx book = _library->findByIsbn(args.front());
	if (!book)
	{
	    cout << "no book with that ISBN number exists." << endl;
	}
	else
	{
	    _foundBooks.push_back(book);
	    _current = _foundBooks.begin();
	    printCurrent();
	}
    }
    catch(const DatabaseException& ex)
    {
	error(ex.message);
    }
    catch(const Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::findAuthors(const list<string>& args)
{
    if (args.size() != 1)
    {
	error("`authors' requires exactly one argument (type `help' for more info)");
	return;
    }

    try
    {
	_foundBooks = _library->findByAuthors(args.front());
	_current = _foundBooks.begin();
	cout << "number of books found: " << _foundBooks.size() << endl;
	printCurrent();
    }
    catch(const DatabaseException& ex)
    {
	error(ex.message);
    }
    catch(const Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::nextFoundBook()
{
    if (_current != _foundBooks.end())
    {
	++_current;
    }
    printCurrent();
}

void
Parser::printCurrent()
{
    try
    {
	if (_current != _foundBooks.end())
	{
	    BookDescription desc = (*_current)->getBookDescription();
	    string renter;
	    try
	    {
		renter = (*_current)->getRenterName();
	    }
	    catch(const BookNotRentedException&)
	    {
	    }

	    cout << "current book is:" << endl;
	    cout << "isbn: " << desc.isbn << endl;
	    cout << "title: " << desc.title << endl;
	    cout << "authors: " << desc.authors << endl;
	    if (!renter.empty())
	    {
		cout << "rented: " << renter << endl;
	    }
	}
	else
	{
	    cout << "no current book" << endl;
	}
    }
    catch(const Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::rentCurrent(const list<string>& args)
{
    if (args.size() != 1)
    {
	error("`rent' requires exactly one argument (type `help' for more info)");
	return;
    }

    try
    {
	if (_current != _foundBooks.end())
	{
	    (*_current)->rentBook(args.front());
	    cout << "the book is now rented by `" << args.front() << "'" << endl;
	}
	else
	{
	    cout << "no current book" << endl;
	}
    }
    catch(const BookRentedException&)
    {
	cout << "the book has already been rented." << endl;
    }
    catch(const DatabaseException& ex)
    {
	error(ex.message);
    }
    catch(const Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::returnCurrent()
{
    try
    {
	if (_current != _foundBooks.end())
	{
	    (*_current)->returnBook();
	    cout << "the book has been returned." << endl;
	}
	else
	{
	    cout << "no current book" << endl;
	}
    }
    catch(const BookNotRentedException&)
    {
	cout << "the book is not currently rented." << endl;
    }
    catch(const DatabaseException& ex)
    {
	error(ex.message);
    }
    catch(const Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::removeCurrent()
{
    try
    {
	if (_current != _foundBooks.end())
	{
	    (*_current)->destroy();
	    cout << "removed current book" << endl;
	}
	else
	{
	    cout << "no current book" << endl;
	}
    }
    catch(const DatabaseException& ex)
    {
	error(ex.message);
    }
    catch(const Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::setEvictorSize(const list<string>& args)
{
    if (args.size() != 1)
    {
	error("`size' requires exactly one argument (type `help' for more info)");
	return;
    }

    try
    {
	_library->setEvictorSize(atoi(args.front().c_str()));
    }
    catch(const DatabaseException& ex)
    {
	error(ex.message);
    }
    catch(const Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::shutdown()
{
    try
    {
	_library->shutdown();
    }
    catch(const Exception& ex)
    {
	ostringstream s;
	s << ex;
	error(s.str());
    }
}

void
Parser::getInput(char* buf, int& result, int maxSize)
{
    if (!_commands.empty())
    {
	if (_commands == ";")
	{
	    result = 0;
	}
	else
	{
#if defined(_MSC_VER) && !defined(_STLP_MSVC)
	    // COMPILERBUG: Stupid Visual C++ defines min and max as macros
	    result = _MIN(maxSize, static_cast<int>(_commands.length()));
#else
	    result = min(maxSize, static_cast<int>(_commands.length()));
#endif
	    strncpy(buf, _commands.c_str(), result);
	    _commands.erase(0, result);
	    if (_commands.empty())
	    {
		_commands = ";";
	    }
	}
    }
    else if (isatty(fileno(yyin)))
    {
#ifdef HAVE_READLINE

	char* line = readline(parser->getPrompt());
	if (!line)
	{
	    result = 0;
	}
	else
	{
	    if (*line)
	    {
		add_history(line);
	    }

	    result = strlen(line) + 1;
	    if (result > maxSize)
	    {
		free(line);
		error("input line too long");
		result = 0;
	    }
	    else
	    {
		strcpy(buf, line);
		strcat(buf, "\n");
		free(line);
	    }
	}

#else

	cout << parser->getPrompt() << flush;

	string line;
	while (true)
	{
	    char c = static_cast<char>(getc(yyin));
	    if (c == EOF)
	    {
		if (line.size())
		{
		    line += '\n';
		}
		break;
	    }

	    line += c;

	    if (c == '\n')
	    {
		break;
	    }
	}
	
	result = line.length();
	if (result > maxSize)
	{
	    error("input line too long");
	    buf[0] = EOF;
	    result = 1;
	}
	else
	{
	    strcpy(buf, line.c_str());
	}

#endif
    }
    else
    {
	if (((result = fread(buf, 1, maxSize, yyin)) == 0) && ferror(yyin))
	{
	    error("input in flex scanner failed");
	    buf[0] = EOF;
	    result = 1;
	}
    }
}

void
Parser::nextLine()
{
    _currentLine++;
}

void
Parser::continueLine()
{
    _continue = true;
}

char*
Parser::getPrompt()
{
    assert(_commands.empty() && isatty(fileno(yyin)));

    if (_continue)
    {
	_continue = false;
	return "(cont) ";
    }
    else
    {
	return ">>> ";
    }
}

void
Parser::error(const char* s)
{
    if (_commands.empty() && !isatty(fileno(yyin)))
    {
	cerr << _currentFile << ':' << _currentLine << ": " << s << endl;
    }
    else
    {
	cerr << "error: " << s << endl;
    }
    _errors++;
}

void
Parser::error(const string& s)
{
    error(s.c_str());
}

void
Parser::warning(const char* s)
{
    if (_commands.empty() && !isatty(fileno(yyin)))
    {
	cerr << _currentFile << ':' << _currentLine << ": warning: " << s << endl;
    }
    else
    {
	cerr << "warning: " << s << endl;
    }
}

void
Parser::warning(const string& s)
{
    warning(s.c_str());
}

int
Parser::parse(FILE* file, bool debug)
{
    extern int yydebug;
    yydebug = debug ? 1 : 0;

    assert(!parser);
    parser = this;

    _errors = 0;
    _commands.empty();
    yyin = file;
    assert(yyin);

    _currentFile = "";
    _currentLine = 0;
    _continue = false;
    nextLine();

    _foundBooks.clear();
    _current = _foundBooks.end();

    int status = yyparse();
    if (_errors)
    {
	status = EXIT_FAILURE;
    }

    parser = 0;
    return status;
}

int
Parser::parse(const string& commands, bool debug)
{
    extern int yydebug;
    yydebug = debug ? 1 : 0;

    assert(!parser);
    parser = this;

    _errors = 0;
    _commands = commands;
    assert(!_commands.empty());
    yyin = 0;

    _currentFile = "<command line>";
    _currentLine = 0;
    _continue = false;
    nextLine();

    _foundBooks.clear();
    _current = _foundBooks.end();

    int status = yyparse();
    if (_errors)
    {
	status = EXIT_FAILURE;
    }

    parser = 0;
    return status;
}

Parser::Parser(const CommunicatorPtr& communicator, const LibraryPrx& library) :
    _communicator(communicator),
    _library(library)
{
}
