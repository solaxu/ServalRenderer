#pragma once

template<typename Value>
class ListNode
{
public:
	Value v;
	ListNode<Value>* next = nullptr;
	ListNode<Value>* prev = nullptr;
	ListNode() : next(nullptr), prev(nullptr) {}
	ListNode(const Value& _v) : next(nullptr), prev(nullptr), v(_v) {}
	~ListNode() {}
	void RemoveSelf()
	{
		if (prev != nullptr)
			prev->next = next;
		if (next != nullptr)
			next->prev = prev;
	}
};

template<typename Value>
class DoubleLinkedList
{
protected:
	ListNode<Value>* front;
	ListNode<Value>* back;
	int length = 0;
public:
	DoubleLinkedList() : front(nullptr), back(nullptr), length(0) {}
	~DoubleLinkedList()
	{
		ListNode<Value>* h = front;
		while (h != nullptr)
		{
			ListNode<Value>* n = h;
			h = h->next;
			n->RemoveSelf();
			delete n;
		}
	}

	Value* Front()
	{
		return &front->v;
	}

	Value* Back()
	{
		return &back->v;
	}

	int GetLength()
	{
		return length;
	}

	void InsertBack(const Value& v)
	{
		ListNode<Value>* n = new ListNode<Value>(v);
		if (back == nullptr)
		{
			front = n;
			back = n;
		}
		else
		{
			back->next = n;
			n->prev = back;
			back = n;
		}
		length++;
	}

	void InsertFront(const Value& v)
	{
		ListNode<Value>* n = new ListNode<Value>(v);
		n->k = k;
		if (front == nullptr)
		{
			front = n;
			back = n;
		}
		else (front != nullptr)
		{
			n->next = front;
			front->prev = n;
			front = n;
		}
		length++;
	}

	void Remove(const Value& v)
	{
		ListNode<Value>* h = front;
		while (h != nullptr)
		{
			if (v == h->v)
				break;
			h = h->next;
		}
		h->RemoveSelf();
		delete h;
		length--;
	}

	void PopFront()
	{
		ListNode<Value>* n = front;
		if (front != nullptr)
		{
			front = front->next;
			if (front == nullptr)
			{
				front = back = nullptr;
			}
		}
		else
		{	
			return;
		}
		n->RemoveSelf();
		length--;
	}

	void PopBack()
	{
		ListNode<Value>* n = back;
		if (back != nullptr)
		{
			back = back->prev;
			if (back == nullptr)
			{
				front = back = nullptr;
			}
		}
		else
		{
			return;
		}
		n->RemoveSelf();
		length--;
	}
};
