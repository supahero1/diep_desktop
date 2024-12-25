#include <DiepDesktop/server/sort.h>
#include <DiepDesktop/shared/debug.h>


Static void
Swap(
	uint16_t* A,
	uint16_t* B
	)
{
	uint16_t Temp = *A;
	*A = *B;
	*B = Temp;
}


Static int32_t
Partition(
	uint16_t* Array,
	int32_t Low,
	int32_t High
	)
{
	uint16_t X = Array[High];
	int32_t j = Low - 1;

	for(int32_t i = Low; i <= High - 1; ++i)
	{
		if(Array[i] <= X)
		{
			++j;
			Swap(Array + j, Array + i);
		}
	}

	++j;
	Swap(Array + j, Array + High);

	return j;
}



void
QuickSort(
	uint16_t* Array,
	int32_t Length
	)
{
	if(Length <= 1)
	{
		return;
	}

	int32_t Low = 0;
	int32_t High = Length - 1;

	int32_t Stack[High - Low + 1];
	Stack[0] = Low;
	Stack[1] = High;

	int32_t* Top = Stack + 2;

	do
	{
		High = *(--Top);
		Low = *(--Top);

		int32_t Pivot = Partition(Array, Low, High);

		if(Pivot - 1 > Low)
		{
			*(Top++) = Low;
			*(Top++) = Pivot - 1;
		}

		if(Pivot + 1 < High)
		{
			*(Top++) = Pivot + 1;
			*(Top++) = High;
		}
	}
	while(Top != Stack);
}
