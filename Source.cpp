#include <iostream>
#include <fstream>
#include <Windows.h>
#include<string>

//Естественное многопутевое двухфазное равномерное
const size_t N = 3;
struct info_cnt
{
	int eor;
	int eof;
	info_cnt(int x = 0, int y = 0) :eor(x), eof(y) {};
};
info_cnt cnt;

struct info
{
	int num;
	char ch;
	info(int x = 0, int y = 0) :num(x), ch(y) {};
};
struct Sequence
{
	std::fstream file;
	info elem;
	bool eof, eor;
	int last=-1;
	Sequence() :eof(0), eor(0),last(-1) {};
	void ReadNext();
	void StartRead(std::string filename);
	void StartWrite(std::string filename);
	void Close();
	void Copy(Sequence& x);
	bool CopyRun(Sequence& x);
};

void Sequence::ReadNext()
{
	if(!file.read((char*)&elem, sizeof(info)))
		eof = true;
}

void Sequence::StartRead(std::string filename)
{
	eof = 0; eor = 0; last = -1;
	file.open(filename, std::ios::in| std::ios::binary);
	ReadNext();
	eor = eof;
}

void Sequence::StartWrite(std::string filename)
{
	eof = 0; eor = 0; last = -1;
	file.open(filename, std::ios::out|std::ios::binary);
}

void Sequence::Close()
{
	file.close();
}

void Sequence::Copy(Sequence& x)
{
	elem = x.elem;
	file.write((char*)&elem,sizeof(info));
	last = elem.num;
	x.ReadNext();
	x.eor = x.eof || x.elem.num < elem.num;
	if (x.eor) 
		cnt.eor--;
	if (x.eof)
		cnt.eof--;
}

bool Sequence::CopyRun(Sequence& x)
{
	bool check = false;
	if (last!=-1&&last<= x.elem.num)
	{
		check = true;
		x.eor = false;
	}
	do
	{
		Copy(x);
	} while (!x.eor);
	return check;
}

void Print(std::string str)
{
	std::ifstream file(str, std::ios::binary);
	info x; int endle = 0;
	while (file.read((char*)&x, sizeof(info)))
	{
		std::cout<< (endle++ % 7 == 0?'\n':'\t') << x.num << ' ' << x.ch << ' ';
	}
	std::cout << '\n';
	file.close();
}

void Distribute(Sequence& f0, std::string filename, Sequence*& f)
{
	f0.StartRead(filename);
	for (int i = 0; i < N; i++)
	{
		std::string str = 'f' + std::to_string(i) + ".bin";
		f[i].StartWrite(str);
	}
	while (!f0.eof)
	{
		for (int i = 0; i < N && !f0.eof;)
		{
			bool check = f[i].CopyRun(f0);
			if (!check)
				i++;
		}
	}
	f0.Close();
	for (int i = 0; i < N; i++)
		f[i].Close();
}



int Find_place_min(Sequence*& f, info_cnt&cnt)
{
	int ind = -1, min = 0;
	for (int i = 0; i < N; i++)
	{
		if (!f[i].eor&& !f[i].eof)
		{
			if (ind == -1)
			{
				min = f[i].elem.num;
				ind = i;
			}
			else if (min > f[i].elem.num)
			{
				ind = i;
				min = f[i].elem.num;
			}
		}
	}
	return ind;
}
int Merge(Sequence& f0, std::string filename, Sequence*& f)
{
	for (int i = 0; i < N; i++)
	{
		std::string str = 'f' + std::to_string(i) + ".bin";
		f[i].StartRead(str);
	}
	f0.StartWrite(filename);
	cnt.eof = 0;
	for(int i=0;i<N;i++)
		if(!f[i].eof)
			cnt.eof++;
	int ind=Find_place_min(f,cnt);
	int count = 0; 

	while (cnt.eof>1)
	{
		cnt.eor = cnt.eof;
		while (cnt.eor>1)
		{
			f0.Copy(f[ind]);
			ind = Find_place_min(f, cnt);
		}
		if (!f[ind].eor)
			f0.CopyRun(f[ind]);

		for(int i=0;i<N;i++)
			f[i].eor = f[i].eof;
		++count;
	}

	ind = Find_place_min(f, cnt);
	while (!f[ind].eof)
	{
		f0.CopyRun(f[ind]);
		++count;
	}
	
	f0.Close();
	for (int i = 0; i < N; i++)
		f[i].Close();
	return count;
}

void Sorting(std::string filename)
{
	Sequence f0, *f= new Sequence[N];
	int count = 0;
	do
	{
		Distribute(f0, filename, f);
		count = Merge(f0, filename, f);
	} while (count > 1);
	
	delete[] f;
}

void CreateFile_(std::string filename)
{
	std::ofstream file(filename);
	srand(GetTickCount());
	info x;//[12] = {20, 3, 1, 6, 8, 9, 3, 5, 0, 35, 2, 46};
	for (int i = 0; i < 100; ++i)
	{
		x.num = rand() % 10;
		x.ch = char('a' + rand() % ('z' - 'a'));
		file.write((char*)&x, sizeof(info));
	}
	file.close();
	//Print(filename);
}
bool CheckFile(std::string filename)
{
	std::ifstream file(filename,std::ios::binary);
	info x, y;
	bool check = true;
	
	while (file.read((char*)&y, sizeof(info)) && check)
	{
		x = y;
		file.read((char*)&y, sizeof(info));
		check = x.num <= y.num;
	}
	file.close();
	return check;
}
int main()
{
	SetConsoleOutputCP(1251);
	CreateFile_("data.bin");

	Sorting("data.bin");
	if (CheckFile("data.bin"))
		std::cout << "Файл упорядочен!\n";
	else
		std::cout << "Файл не упорядочен!\n";
	Print("data.bin");
	std::cin.get();
	return 0;
}