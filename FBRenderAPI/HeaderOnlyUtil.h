#pragma once
#include <cstdint>
namespace fb
{
	class FRefCountBase
	{
	public:
		FRefCountBase() = default;
		virtual ~FRefCountBase() = default;

		FRefCountBase(const FRefCountBase& Rhs) = delete;
		FRefCountBase& operator=(const FRefCountBase& Rhs) = delete;

		inline int32_t AddRef() const
		{
			return ++NumRefs;
		}

		inline int32_t Release() const
		{
			--NumRefs;	
			if (NumRefs == 0)
			{
				delete this;
			}

			return NumRefs;
		}

		int32_t GetRefCount() const
		{
			return NumRefs;
		}

	private:
		mutable int32_t NumRefs = 0;

		void CheckRefCount() const;
	};

	//
	//  intrusive_ptr
	//
	//  A smart pointer that uses intrusive reference counting.
	//
	//  Relies on unqualified calls to
	//  
	//      void intrusive_ptr_add_ref(T * p);
	//      void intrusive_ptr_release(T * p);
	//
	//          (p != 0)
	//
	//  The object is responsible for destroying itself.
	//
	template<class T> class intrusive_ptr
	{
	private:

		typedef intrusive_ptr this_type;

	public:

		typedef T element_type;

		constexpr intrusive_ptr() noexcept : px(0)
		{
		}

		intrusive_ptr(T* p, bool add_ref = true) : px(p)
		{
			if (px != 0 && add_ref) intrusive_ptr_add_ref(px);
		}

		template<class U>
		intrusive_ptr(intrusive_ptr<U> const& rhs, std::enable_if_t<std::is_convertible_v<U, T>>)
			: px(rhs.get())
		{
			if (px != 0) intrusive_ptr_add_ref(px);
		}

		intrusive_ptr(intrusive_ptr const& rhs) : px(rhs.px)
		{
			if (px != 0) intrusive_ptr_add_ref(px);
		}

		~intrusive_ptr()
		{
			if (px != 0) intrusive_ptr_release(px);
		}

		template<class U> intrusive_ptr& operator=(intrusive_ptr<U> const& rhs)
		{
			this_type(rhs).swap(*this);
			return *this;
		}

		// Move support
		intrusive_ptr(intrusive_ptr&& rhs) noexcept : px(rhs.px)
		{
			rhs.px = 0;
		}

		intrusive_ptr& operator=(intrusive_ptr&& rhs) noexcept
		{
			this_type(static_cast<intrusive_ptr&&>(rhs)).swap(*this);
			return *this;
		}

		template<class U> friend class intrusive_ptr;

		template<class U>
		intrusive_ptr(intrusive_ptr<U>&& rhs, std::enable_if<!std::is_convertible_v<U, T>>)
			: px(rhs.px)
		{
			rhs.px = 0;
		}

		template<class U>
		intrusive_ptr& operator=(intrusive_ptr<U>&& rhs) noexcept
		{
			this_type(static_cast<intrusive_ptr<U>&&>(rhs)).swap(*this);
			return *this;
		}

		intrusive_ptr& operator=(intrusive_ptr const& rhs)
		{
			this_type(rhs).swap(*this);
			return *this;
		}

		intrusive_ptr& operator=(T* rhs)
		{
			this_type(rhs).swap(*this);
			return *this;
		}

		void reset()
		{
			this_type().swap(*this);
		}

		void reset(T* rhs)
		{
			this_type(rhs).swap(*this);
		}

		void reset(T* rhs, bool add_ref)
		{
			this_type(rhs, add_ref).swap(*this);
		}

		T* get() const noexcept
		{
			return px;
		}

		T* detach() noexcept
		{
			T* ret = px;
			px = 0;
			return ret;
		}

		T& operator*() const noexcept
		{
			assert(px != 0);
			return *px;
		}

		T* operator->() const noexcept
		{
			assert(px != 0);
			return px;
		}

		// implicit conversion to "bool"
		explicit operator bool() const noexcept
		{
			return px != 0;
		}

		bool operator! () const noexcept
		{
			return px == 0;
		}


		void swap(intrusive_ptr& rhs) noexcept
		{
			T* tmp = px;
			px = rhs.px;
			rhs.px = tmp;
		}

	private:

		T* px;
	};

	template<class T, class U> inline bool operator==(intrusive_ptr<T> const& a, intrusive_ptr<U> const& b) noexcept
	{
		return a.get() == b.get();
	}

	template<class T, class U> inline bool operator!=(intrusive_ptr<T> const& a, intrusive_ptr<U> const& b) noexcept
	{
		return a.get() != b.get();
	}

	template<class T, class U> inline bool operator==(intrusive_ptr<T> const& a, U* b) noexcept
	{
		return a.get() == b;
	}

	template<class T, class U> inline bool operator!=(intrusive_ptr<T> const& a, U* b) noexcept
	{
		return a.get() != b;
	}

	template<class T, class U> inline bool operator==(T* a, intrusive_ptr<U> const& b) noexcept
	{
		return a == b.get();
	}

	template<class T, class U> inline bool operator!=(T* a, intrusive_ptr<U> const& b) noexcept
	{
		return a != b.get();
	}



	template<class T> inline bool operator==(intrusive_ptr<T> const& p, std::nullptr_t) noexcept
	{
		return p.get() == 0;
	}

	template<class T> inline bool operator==(std::nullptr_t, intrusive_ptr<T> const& p) noexcept
	{
		return p.get() == 0;
	}

	template<class T> inline bool operator!=(intrusive_ptr<T> const& p, std::nullptr_t) noexcept
	{
		return p.get() != 0;
	}

	template<class T> inline bool operator!=(std::nullptr_t, intrusive_ptr<T> const& p) noexcept
	{
		return p.get() != 0;
	}

	template<class T> inline bool operator<(intrusive_ptr<T> const& a, intrusive_ptr<T> const& b) noexcept
	{
		return std::less<T*>()(a.get(), b.get());
	}

	template<class T> void swap(intrusive_ptr<T>& lhs, intrusive_ptr<T>& rhs) noexcept
	{
		lhs.swap(rhs);
	}

	// mem_fn support

	template<class T> T* get_pointer(intrusive_ptr<T> const& p) noexcept
	{
		return p.get();
	}

	template<class T, class U> intrusive_ptr<T> static_pointer_cast(intrusive_ptr<U> const& p)
	{
		return static_cast<T*>(p.get());
	}

	template<class T, class U> intrusive_ptr<T> const_pointer_cast(intrusive_ptr<U> const& p)
	{
		return const_cast<T*>(p.get());
	}

	template<class T, class U> intrusive_ptr<T> dynamic_pointer_cast(intrusive_ptr<U> const& p)
	{
		return dynamic_cast<T*>(p.get());
	}

	// operator<<


	template<class E, class T, class Y> std::basic_ostream<E, T>& operator<< (std::basic_ostream<E, T>& os, intrusive_ptr<Y> const& p)

	{
		os << p.get();
		return os;
	}
}