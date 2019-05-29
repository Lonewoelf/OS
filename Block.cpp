#include "block.h"

Block::Block(const char *name, int number)
{
	this->name = name;
	this->number = number;
}

Block::~Block()
{
}

// Operator overloading voor de fijnprovers
std::ostream &operator << (std::ostream &out, Block *b)
{
	return out << b->name << "-" << b->number;
}
