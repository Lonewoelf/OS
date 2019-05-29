#include <ostream>

class Block {
public:
	Block(const char *name, int number);
	virtual ~Block();

	friend std::ostream &operator << (std::ostream &out, Block *b);

private:
	const char *name;
	int number;
};