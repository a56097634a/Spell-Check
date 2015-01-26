#!/usr/local/bin/thrift --gen cpp

namespace cpp SpellServer
namespace py spell
struct SpellRequest{
       1:list<string>to_check
}

struct SpellResponse{
       1:list<bool>is_correct
}

service SpellService{
	SpellResponse spellcheck(1:SpellRequest request)
}

