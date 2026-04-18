#include <cassert>
#include <cstdlib>
#include <iostream>

template <typename T> class unique_ptr {
		T *mPtr;
		unique_ptr(const unique_ptr&);
		unique_ptr operator=(const unique_ptr&);

	public:
		unique_ptr() {};
		unique_ptr(T *ptr): mPtr(ptr) {};

		unique_ptr(unique_ptr& src): mPtr(src.mPtr) { src.mPtr = NULL; }

		unique_ptr& operator=(unique_ptr& src)
		{
			delete mPtr;
			mPtr		 = src.mPtr;
			src.mPtr = NULL;

			return *this;
		}

		~unique_ptr()
		{
			std::cout << ">> destroying unique_ptr" << std::endl;
			delete mPtr;
		}

		T *operator->() { return mPtr; }
		T *getPtr() const { return mPtr; }
};

int main()
{
	int						 *a = new int(5);
	unique_ptr<int> ptr(a);
	int							b = 42;

	std::cout << "ptr: " << *ptr.getPtr() << std::endl;

	return (0);
}
