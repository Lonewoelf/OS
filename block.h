#include <ostream>

class Block {
public:
	Block(signed short inputBuff, int id);
	virtual ~Block();

	signed short getSample();
	void setStatus(bool);
	bool getStatus();

	friend std::ostream &operator << (std::ostream &out, Block *b);

private:
	signed short inputBuff;
	bool status;
	int id;
};