#include <iostream>
#include <vector>

class Int {
	public:
		Int(): num(0) {};
		Int(int i): num(i) {};
		Int(const Int& other) { (void)other; };
		Int& operator=(const Int& other)
		{
			(void)other;
			return (*this);
		};
		~Int() {};

		int num;
};

class Bla {
	public:
		Bla() { bl.push_back(Int()); };
		Bla(const Bla& other) { (void)other; };
		Bla& operator=(const Bla& other)
		{
			(void)other;
			return (*this);
		};
		~Bla() {};

		void						 vf();
		std::vector<Int> bl;
};

void Bla::vf()
{
	Int moep(100);
	moep.num = 10;
	bl.push_back(moep);
};

// the output will be
//
//		 blub.bl[0]: 0
//		 blub.bl[1]: 0
//
// because we did not yet implement the copy constructor for Bla class
int main()
{
	Bla blub;
	blub.vf();
	for (size_t i = 0; i < blub.bl.size(); i++) {
		std::cout << "blub.bl[" << i << "]: " << blub.bl[i].num << std::endl;
	}
}
