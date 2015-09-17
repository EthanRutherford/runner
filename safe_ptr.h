#ifndef SAFE_PTR_H
#define SAFE_PTR_H

class safe_ptr_b{
	public:
		virtual void* get() = 0;
		virtual void forget() = 0;
		virtual ~safe_ptr_b() {}
};
template<class T>
class safe_ptr_i : public safe_ptr_b{
	public:
		safe_ptr_i(T* p) { ptr = p; }
		virtual void forget() { ptr = NULL; }
		virtual void* get() { return ptr; }
		virtual ~safe_ptr_i() { if (ptr != NULL) delete ptr;  }
	private:
		T* ptr;
};

class safe_ptr{
	public:
		safe_ptr() { base = NULL; }
		template<class T>
		safe_ptr(T* ptr) {
			base = new safe_ptr_i<T>(ptr);
		}
		template<class T>
		void operator=(T* ptr) {
			if (base != NULL)
			{
				base->forget();
				delete base;
			}
			base = new safe_ptr_i<T>(ptr);
		}
		template<class T>
		operator T() { return reinterpret_cast<T>(base->get()); }
		void operator delete(void* p) { delete reinterpret_cast<safe_ptr*>(p)->base; }
	private:
		safe_ptr_b* base;
};

#endif