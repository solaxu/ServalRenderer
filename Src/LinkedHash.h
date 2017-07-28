#pragma once

// key should be an integer
// if we can hash a string at compile time, we can use string as a Key value. Gong Minmin has had a talking on this topic at Zhihu.

template<typename Value>
class HashNode
{
public:
	int k;
	Value v;
	HashNode<Value>* next = nullptr;
	HashNode<Value>* prev = nullptr;
	HashNode() : next(nullptr), prev(nullptr), k(0) {}
	HashNode(const Value& _v) : next(nullptr), prev(nullptr), k(0), v(_v) {}
	~HashNode() {}
	void RemoveSelf()
	{
		if (prev != nullptr)
			prev->next = next;
		if (next != nullptr)
			next->prev = prev;
	}
};

template<typename Value>
class LinkedHash
{
protected:
	HashNode<Value> table[100];
public:
	LinkedHash() {}
	~LinkedHash() 
	{
		for (int i = 0; i < 100; ++i)
		{
			HashNode<Value>* h = &table[i];
			while (h->next != nullptr)
			{
				HashNode<Value>* n = h->next;
				h->next->RemoveSelf();
				delete n;
			}
		}
	}
	void Insert(int k, const Value& v)
	{
		int slot_index = k % 100;
		// head insert a HashNode into a double linked list
		HashNode<Value>* h = &table[slot_index];
		HashNode<Value>* n = new HashNode<Value>(v);
		n->k = k;
		n->next = h->next;
		if (h->next != nullptr)
			h->next->prev = n;
		n->prev = h;
		h->next = n;
	}

	void Remove(int k)
	{
		int slot_index = k % 100;
		// head insert a HashNode into a double linked list
		HashNode<Value>* h = table[slot_index].next;
		while (h != nullptr)
		{
			if (k == h->k)
				break;
			h = h->next;
		}
		h->RemoveSelf();
		delete h;
	}

	const Value* GetValuePtr(int k)
	{
		int slot_index = k % 100;
		// head insert a HashNode into a double linked list
		HashNode<Value>* h = table[slot_index].next;
		while (h != nullptr)
		{
			if (k == h->k)
				return &h->v;
			h = h->next;
		}
		return nullptr;
	}
};