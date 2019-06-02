#include <ostream>

class Block {
public:
	Block(signed short inputBuff, int id);
	virtual ~Block();

	friend std::ostream &operator << (std::ostream &out, Block *b);

private:
	signed short inputBuff;
	int id;
};