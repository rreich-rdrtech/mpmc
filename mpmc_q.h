#include <atomic>
#include <cassert>
#include <iostream>

template<typename T>
class mpmc_queue
{

inline bool is_pow2(uint64_t s)
{
	return ((s >= 2) && !(s & (s - 1)));
}

public:
/*
  void init_shm(uint64_t q_elements, void* buffer)
{
	m_q_mem = reinterpret_cast<Cell_t*>(buffer);
	m_q_pos_mask_ = q_elements - 1;
	assert((q_elements >= 2) && ((q_elements & (q_elements - 1)) == 0));

	shm = true;

	for (uint64_t i = 0; i != q_elements; i += 1)
		m_q_mem[i].m_seq.store(i, std::memory_order_relaxed);

	m_enq_pos.store(0, std::memory_order_relaxed);
	m_deq_pos.store(0, std::memory_order_relaxed);
}
*/
static uint64_t GetSize ( uint64_t elements )
{
	uint64_t cell_size = sizeof(Cell_t) * elements;
	uint64_t q_struct_size = sizeof(queue);

	return cell_size + q_struct_size;
}


mpmc_queue(uint64_t q_elements)
	: m_q_pos_mask_(q_elements - 1)
	, m_q_mem(new Cell_t [q_elements])
{
	assert(is_pow2(q_elements));

	m_q = new queue;

	for (uint64_t i = 0; i != q_elements; ++i)
		m_q_mem[i].m_done.store(false, std::memory_order_relaxed);

	m_q->m_enq_pos.store(0, std::memory_order_relaxed);

	m_q->m_deq_pos.store(0, std::memory_order_relaxed);

	std::cout << "Size of Cell_t = " << sizeof(Cell_t) << std::endl;
}

~mpmc_queue()
{
	//delete [] m_q_mem;
}

bool push(const T& data)
{
	Cell_t* cell;

	for(;;)
	{
		uint64_t pos = m_q->m_enq_pos.load(std::memory_order_acquire);
		cell = &(m_q_mem[pos & m_q_pos_mask_]);

		if (cell->m_done.load(std::memory_order_relaxed) == true)
			return false;

		if (!m_q->m_enq_pos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
			continue;

		cell->m_data = data;
		cell->m_done.store(true, std::memory_order_release);

		return true;
	}
}

bool pop(T& data)
{
	Cell_t* cell;

	for(;;)
	{
		uint64_t pos = m_q->m_deq_pos.load(std::memory_order_acquire);
		cell = &(m_q_mem[pos & m_q_pos_mask_]);

		if (cell->m_done.load(std::memory_order_relaxed) == false)
			return false;
	
		if (!m_q->m_deq_pos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
			continue;

		data = cell->m_data;
		cell->m_done.store(false, std::memory_order_release);

		return true;
	}
}

public:
	struct queue
	{
		alignas(alignof(T)) std::atomic<uint64_t>   m_enq_pos;
		alignas(alignof(T)) char x1; // shadow false sharing
		alignas(alignof(T)) std::atomic<uint64_t>   m_deq_pos;
		alignas(alignof(T)) char x2; // shadow false sharing?
	};

private:
	struct alignas(alignof(T)) Cell_t
	{
		std::atomic<bool>		m_done{false};
		T						m_data;
	};

	const uint64_t		m_q_pos_mask_;
	Cell_t*				m_q_mem;
	queue*				m_q{nullptr};

	mpmc_queue(const mpmc_queue&) = delete;
	void operator = (const mpmc_queue&) = delete;
}; 
