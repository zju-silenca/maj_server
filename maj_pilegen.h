#ifndef _MAJ_PILEGEN_H
#define _MAJ_PILEGEN_H
#include <iostream>
#include <vector>
#include <random>
#include <muduo/net/TcpServer.h>

using namespace std;

class MajPilegen
{
public:
    MajPilegen(int model = 4):model_(model)
    {
        if(model == 4)
        {
            majpile_.reserve(136);
            initializePile4();
            upsetPile();
        }
    }

    string getMaj(int no)
    {
        return majpile_[no];
    }

    string dealMaj()
    {
        if(dealNum_ < static_cast<int>(majpile_.size()))
            return majpile_[dealNum_++];
        else
            return "  ";
    }
    /*for test
    void printAll()
    {
        for(int i = 0; i < 136; i++)
        {
            cout << majpile_[i] << ' ';
        }
        cout << endl;
    }
    */
    

    //生成牌山
    void initializePile4()
    {
        majpile_.clear();
        for(int i = 1; i < 10; i++)
        {
            for(int j = 0; j < 4; j++)
            {
                majpile_.push_back(to_string(i)+"m");//万
                majpile_.push_back(to_string(i)+"p");//饼
                majpile_.push_back(to_string(i)+"s");//索
                if(i <= 7)
                {
                    majpile_.push_back(to_string(i)+"z");//7种字牌
                }
            }
        }
    }
    //打乱牌山
    void upsetPile()
    {
        dealNum_ = 0;
        mountNum_ = 4;
        int64_t seed = muduo::Timestamp::now().microSecondsSinceEpoch();
        default_random_engine e(seed);
        //cout << "打乱牌山随机种子:" << seed << endl;
        uniform_int_distribution<int> u(1, 135);
        int size_ = static_cast<int>(majpile_.size());
        for(int i = 0; i < size_*size_; i++)
        {
            swap(majpile_[0],majpile_[u(e)]);
        }
    }
private:
    //模式代码，目前先实现四麻
    int model_;
    int dealNum_;
    int mountNum_;
    vector<string> majpile_;
};


#endif //_MAJ_PILEGEN_H