// ConsoleApplication1.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include <iostream>

using namespace std;

// Duck을 슈퍼클래스로 두고, 이를 상속받는 각종 오리들을 만들어보자.
// 1. 꽥소리를 내는 오리 / 삑 소리를 내는 오리 / 안우는 오리가 있다.
// 2. 나는 오리 / 날지못하는 오리가 있다.

// 오리의 우는 행동
class QuackBehavior
{
public:
	virtual void quack()
	{}
};

// 오리의 우는 행동 - 꽥 소리
class Quack	:public QuackBehavior
{
public:
	virtual void quack()
	{
		cout << "꿕꿕오리 : 꿕꿕!!!!" << endl;
	}
};

// 오리의 우는 행동 - 삑 소리
class Squeak :public QuackBehavior
{
public:
	virtual void quack()
	{
		cout << "삑삑오리 : 삑삑삑삑삑삑!!!!" << endl;
	}

};

// 오리의 우는 행동 - 안 운다.
class MuteQuack :public QuackBehavior
{
public:
	virtual void quack()
	{
		cout << "뮤트오리 : ....." << endl;
	}
};


// 오리의 나는 행동
class FlyBehavior
{
public:
	virtual void fly()
	{}
};

// 오리의 나는 행동 - 날다
class FlyWithWIngs	:public FlyBehavior
{
public:
	virtual void fly()
	{
		cout << "Fly!!!~" << endl;
	}
};

// 오리의 나는 행동 - 안날다
class FlyNoWings :public FlyBehavior
{

public:
	virtual void fly()
	{
		cout << "No Fly..." << endl;
	}
};



// 슈퍼클래스
class Duck
{
	QuackBehavior* quackBehavior;
	FlyBehavior* flyBehavior;

public:
	Duck()
	{
		quackBehavior = nullptr;
		flyBehavior = nullptr;
	}
	~Duck()
	{
		if (quackBehavior != nullptr)
			delete quackBehavior;

		if (flyBehavior != nullptr)
			delete flyBehavior;
	}

	// 우는 행동
	void ActionQuack()
	{
		quackBehavior->quack();
	}

	// 나는 행동
	void ActionFly()
	{
		flyBehavior->fly();
	}

protected:
	// QuackBehavior 셋팅
	void SetQuackBehabior(QuackBehavior* qa)
	{
		quackBehavior = qa;
	}

	//flyBehavior 셋팅
	void SetFlyBehavior(FlyBehavior* fy)
	{
		flyBehavior = fy;
	}

};

// 꿕 하고 우는 오리
class QuackDuck	:public Duck
{
public:
	QuackDuck()
	{
		SetQuackBehabior(new Quack);
	}
};

// 삑 하고 우는 오리
class SqueckDuck :public Duck
{
public:
	SqueckDuck()
	{
		SetQuackBehabior(new Squeak);
	}
};

// 안우는 오리
class MuteDuck :public Duck
{
public:
	MuteDuck()
	{
		SetQuackBehabior(new MuteQuack);
	}
};

// 나는 오리
class FlyDuck	:public Duck
{
public:
	FlyDuck()
	{
		SetFlyBehavior(new FlyWithWIngs);
	}
};

int main()
{
	// 꿕 하고 우는 오리
	QuackDuck duck;
	duck.ActionQuack();

	// 삑 하고 우는 오리
	SqueckDuck duck2;
	duck2.ActionQuack();

	// 안우는 오리
	MuteDuck duck3;
	duck3.ActionQuack();

	// 나는 오리
	FlyDuck duck4;
	duck4.ActionFly();


	return 0;
}
