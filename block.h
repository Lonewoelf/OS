#include <ostream>

class Block {
public:
	Block(signed short inputBuff, int id);
	virtual ~Block();

	signed short getSample();
	void setStatus(bool);
	bool getStatus();

private:
	signed short inputBuff;
	bool status;
	int id;
};