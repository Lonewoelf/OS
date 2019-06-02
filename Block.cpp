#include "block.h"

Block::Block(signed short inputBuff, int id)
{
	this->inputBuff = inputBuff;
	this->id = id;
	bool edit = 0;
}

Block::~Block()
{
}

signed short Block::getSample()
{
	return this->inputBuff;
}

void Block::setStatus(bool status)
{
	this->status = status;
}

bool Block::getStatus()
{
	return this->status;
}