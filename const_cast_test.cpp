#include<iostream>
using namespace std;

class A
{
public:
	int m;
	
	A(int l)
	{
		m = l;
	}
};

int main(int , char **)										//C++には、関数の引数に対して、使わない引数の名前はいらない
{
	
	#if 0  
	/****************Test 1**************************************/
	const char *a = "1234";

	char *b = const_cast<char *>(a);

	cout << "a=" << a << "; b= " << b << endl;


	b[0] = 'a';

	cout << "a=" << a << "; b= " << b << endl;
	#endif

	A l = 10;										//もし　コンストラクタA(int l){}がなければ、 『エラー: ‘int’ から非スカラ型 ‘A’ への変換が要求されました』が出る

	const A *a = new A(l);
	//a.m = 100;

	A *b = const_cast<A*>(a);						//const_cast<type_id>(expression)   type_idはポインタ、参照型、もしくはpoint-to-data-memberでなければなりません

	cout << "a.m:" <<a->m << "; b.m= " << b->m << endl;

	//a->m = 300;											//  エラー: 読み取り専用オブジェクト内のメンバ ‘A::m’ への代入です a->m = 300;


	cout << "a.m:" <<a->m << "; b.m= " << b->m << endl;		// 10 10

	b->m = 200;

	cout << "a.m:" <<a->m << "; b.m= " << b->m << endl;    // 200 200

	//int A::*p_m = &A::m;					
	//cout << "a.m:" <<a->m << "; b.m= " << b->m << "a.p_m:" <<*a->p_m << "; b.p_m= " << *b->p_m<< endl;	// エラー: ‘const class A’ has no member named ‘p_m’


	int A::*p_m = &A::m;									//point-to-data-member
	//a->*p_m					
	cout << "a.m:" <<a->m << "; b.m= " << b->m << "a.p_m:" <<a->*p_m << "; b.p_m= " << b->*p_m<< endl;	 //OK


	return 0;
}