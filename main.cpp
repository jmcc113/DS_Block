#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <queue>

#define OK 1
#define ERROR 0
#define OVERFLOW -1

#define TEST 1

typedef int Status;

struct data{//用于数据读取的数组元素对象
    long tx_id;
    double amount;
    int ID;
    char from[35],to[35];
};

typedef struct Trade_Node{//交易信息节点
    long tx_id;
    double amount;
    Trade_Node *nextto,*nextfrom;//十字链表
    char from[35],to[35];
}Trade_Node,*Trade_List;

typedef struct Tree_Node{//仅存储帐号信息的树节点
    Tree_Node *lchild,*rchild;
    Trade_Node *pt;//指向第一个该帐号的交易信息节点
    int ldepth,rdepth,factor;
    char name[35];
}Tree_Node,*Tree;

struct vector{//用于平衡二叉树的辅助元素
    int length;
    int size;
    Tree_Node **p;
    int *dir;
};

Status InitVector(vector &v){//初始化vector
    v.p = (Tree_Node**)malloc(20*sizeof(Tree_Node*));
    v.dir = (int*)malloc(20* sizeof(int));
    if(!v.p||!v.dir)
        exit(OVERFLOW);
    v.length = 0;
    v.size = 20;
    return OK;
}
Status Push(vector &v,Tree_Node *node,int dir){
    if(v.length>=v.size){
        v.p = (Tree_Node**)realloc(v.p,(v.size+20)* sizeof(Tree_Node*));
        v.dir = (int*)realloc(v.dir,(v.size+20)* sizeof(int));
        if(!v.p||!v.dir)
            exit(OVERFLOW);
        v.size += 20;
    }
    v.p[v.length] = node;
    v.dir[v.length] = dir;
    v.length++;
    return OK;
}
Status Pop(vector &v){
    v.length--;
    return OK;
}
Status DestroyVector(vector &v){
    free(v.p);
    free(v.dir);
    return OK;
}
typedef struct Block_Node{//存储区块信息的链表节点
    long time;
    Tree_Node *from,*to;
    Block_Node *next;
    int ID;
}Block_Node,*Block_List;

struct HashMap_B{//用于加快初始化的速度的哈希表
    Block_Node **p;
    int length;
};

Status InsertBlockNode(Block_Node *n,int ID,long time,HashMap_B &H){//插入一个节点
    Block_Node *node;
    node = (Block_Node *) malloc(sizeof(Block_Node));
    if(!node)
        exit(OVERFLOW);
    node->ID = ID;
    node->time = time;
    node->next = NULL;
    node->from = node->to = NULL;
    n->next = node;
    if(H.p[ID-80339]){//链表ID直接-80339 简单哈希
        printf("Conflict!\n");
        exit(OVERFLOW);
    }
    H.length++;
    H.p[ID-80339] = node;
    return OK;
}
Status InitBlockList(Block_List &L,char *file,HashMap_B &H){//初始化
    clock_t start,finish;
    FILE *fp;
    fp = fopen(file,"r");
    if(!fp) {
        printf("No Such File!\n");
        return ERROR;
    }
    printf("Block Initializing...\nLoading...\n");
    start = clock();
    L = (Block_Node*)malloc(sizeof(Block_Node));
    if(!L)
        exit(OVERFLOW);
    L->ID = 0;
    L->time = 0;
    char read[100];
    Block_Node *n = L;
    fgets(read,100,fp);
    while(!feof(fp)){//读文件
        int ID;
        long time;
        fscanf(fp,"%d,%*[^,],%ld\n",&ID,&time);
        InsertBlockNode(n,ID,time,H);
        L->ID++;
        n = n->next;
    }
    finish = clock();
    printf("Complete!\nInitialized Block in %ldms\n\n",(finish-start));
    fclose(fp);
    return OK;
}
Tree_Node* TreeSearch(Tree T,char *name,vector &v){//在平衡二叉树上找目标
    Tree_Node *p = T;//找到返回节点指针，失败返回NULL
    while(1) {
        if(!p)//没找到
            return NULL;
        int st = strcmp(name, p->name);
        if (!st)
            return p;
        else if (st<0) {
            Push(v,p,1);
            p = p->lchild;
        }
        else {
            Push(v,p,2);
            p = p->rchild;
        }
    }
}
Status newDepth(Tree_Node *node){//重新计算节点的左子树和右子树高度以及平衡因子
    if(node->lchild)
        node->ldepth = 1+(node->lchild->ldepth>=node->lchild->rdepth?node->lchild->ldepth:node->lchild->rdepth);
    else
        node->ldepth = 0;
    if(node->rchild)
        node->rdepth = 1+(node->rchild->ldepth>=node->rchild->rdepth?node->rchild->ldepth:node->rchild->rdepth);
    else
        node->rdepth = 0;
    node->factor = node->ldepth-node->rdepth;
    return OK;
}
Tree_Node* Adjust(Tree_Node *node,int mode){//平衡调整
    if(mode==1){//左旋
        Tree_Node *pnode = node->lchild;
        node->lchild = pnode->rchild;
        pnode->rchild = node;
        newDepth(node);
        newDepth(pnode);
        return pnode;
    }
    else if(mode==2){
        Tree_Node *pnode = node->rchild;
        node->rchild = pnode->lchild;
        pnode->lchild = node;
        newDepth(node);
        newDepth(pnode);
        return pnode;
    }
    else if(mode==3){
        Tree_Node *pnode = node->lchild;
        node->lchild = Adjust(pnode,2);
        return Adjust(node,1);
    }
    else{
        Tree_Node *pnode = node->rchild;
        node->rchild = Adjust(pnode,1);
        return Adjust(node,2);
    }
}
Status InsertBTree(Block_Node *p,data *array,int n){//在区块节点上建立一棵树
    if(n==0)
        return OK;
    Tree Tin,Tout;//入树和出树
    Tin = p->to;
    Tout = p->from;
    for(int i=0;i<n;++i){
        Trade_Node *tra;
        tra = (Trade_Node*)malloc(sizeof(Trade_Node));
        if(!tra)
            exit(OVERFLOW);
        tra->tx_id = array[i].tx_id;
        tra->amount = array[i].amount;
        strcpy(tra->from,array[i].from);
        strcpy(tra->to,array[i].to);
        tra->nextfrom = tra->nextto = NULL;
        Tree_Node *tempnodeout,*tempnodein;
        vector v1,v2;
        InitVector(v1);//用来保存搜寻的路径，从而更新平衡树
        InitVector(v2);
        tempnodeout = TreeSearch(Tout, array[i].from,v1);
        tempnodein = TreeSearch(Tin,array[i].to,v2);
        if(!tempnodeout){//没找到就加入新节点
            Tree_Node *nodeout;
            nodeout = (Tree_Node*)malloc(sizeof(Tree_Node));
            if(!nodeout)
                exit(OVERFLOW);
            nodeout->lchild = nodeout->rchild = NULL;
            nodeout->ldepth = nodeout->rdepth = nodeout->factor = 0;
            strcpy(nodeout->name,array[i].from);
            nodeout->pt = tra;
            if(!v1.length)
                Tout = nodeout;
            else {
                int dir;
                if (v1.dir[v1.length - 1] == 1) {//走左边
                    v1.p[v1.length - 1]->lchild = nodeout;
                    v1.p[v1.length - 1]->ldepth++;
                    v1.p[v1.length - 1]->factor++;
                    dir = 1;
                } else {//走右边
                    v1.p[v1.length - 1]->rchild = nodeout;
                    v1.p[v1.length - 1]->rdepth++;
                    v1.p[v1.length - 1]->factor--;
                    dir = 2;
                }
                Pop(v1);
                int done = 0;//是否已经更新平衡树
                while (v1.length > 0) {
                    newDepth(v1.p[v1.length - 1]);//所有节点重新计算高度
                    if (!done && abs(v1.p[v1.length - 1]->factor) > 1) {//失衡
                        Tree_Node *tp;
                        if (v1.dir[v1.length - 1] == 1) {
                            if (dir == 1)
                                tp = Adjust(v1.p[v1.length - 1], 1);
                            else
                                tp = Adjust(v1.p[v1.length - 1], 3);
                        }
                        else {
                            if (dir == 1)
                                tp = Adjust(v1.p[v1.length - 1], 4);
                            else
                                tp = Adjust(v1.p[v1.length - 1], 2);
                        }
                        if (v1.length > 1) {//如果失衡的不是根节点
                            if (v1.dir[v1.length - 2] == 1)
                                v1.p[v1.length - 2]->lchild = tp;
                            else
                                v1.p[v1.length - 2]->rchild = tp;
                        }
                        else
                            Tout = tp;
                        done = 1;
                    }
                    dir = v1.dir[v1.length - 1];
                    Pop(v1);
                }
            }
        }
        else{
            tra->nextfrom = tempnodeout->pt;
            tempnodeout->pt = tra;
        }
        if(!tempnodein){
            Tree_Node *nodein;
            nodein = (Tree_Node*)malloc(sizeof(Tree_Node));
            if(!nodein)
                exit(OVERFLOW);
            nodein->lchild = nodein->rchild = NULL;
            nodein->ldepth = nodein->rdepth = nodein->factor = 0;
            strcpy(nodein->name,array[i].to);
            nodein->pt = tra;
            if(!v2.length)
                Tin = nodein;
            else {
                int dir;
                if (v2.dir[v2.length - 1] == 1) {
                    v2.p[v2.length - 1]->lchild = nodein;
                    v2.p[v2.length - 1]->ldepth++;
                    v2.p[v2.length - 1]->factor++;
                    dir = 1;
                } else {
                    v2.p[v2.length - 1]->rchild = nodein;
                    v2.p[v2.length - 1]->rdepth++;
                    v2.p[v2.length - 1]->factor--;
                    dir = 2;
                }
                int done = 0;
                while (v2.length > 0) {
                    newDepth(v2.p[v2.length - 1]);
                    if (!done && abs(v2.p[v2.length - 1]->factor) > 1) {
                        Tree_Node *tp;
                        if (v2.dir[v2.length - 1] == 1) {
                            if (dir == 1)
                                tp = Adjust(v2.p[v2.length - 1], 1);
                            else
                                tp = Adjust(v2.p[v2.length - 1], 3);
                        } else {
                            if (dir == 1)
                                tp = Adjust(v2.p[v2.length - 1], 4);
                            else
                                tp = Adjust(v2.p[v2.length - 1], 2);
                        }
                        if (v2.length > 1) {
                            if (v2.dir[v2.length - 2] == 1)
                                v2.p[v2.length - 2]->lchild = tp;
                            else
                                v2.p[v2.length - 2]->rchild = tp;
                        } else
                            Tin = tp;
                        done = 1;
                    }
                    dir = v2.dir[v2.length - 1];
                    Pop(v2);
                }
            }
        }
        else{
            tra->nextto = tempnodein->pt;
            tempnodein->pt = tra;
        }
        DestroyVector(v1);
        DestroyVector(v2);
    }
    p->from = Tout;
    p->to = Tin;
    return OK;
}
Status CreateTree(Block_List &L,char *file,HashMap_B H){
    clock_t start,finish;
    FILE *fp;
    fp = fopen(file,"r");
    if(!fp){
        printf("No Such File!\n");
        return ERROR;
    }
    printf("Trade Data Initializing...\nLoading...\n");
    start = clock();
    data *data_array;
    data_array = (data*)malloc(50*sizeof(data));
    if(!data_array)
        exit(OVERFLOW);
    int data_length = 0,data_size = 50;
    Block_Node *p = L->next;
    char read[150];
    fgets(read,150,fp);
    if(read[0]>='0'&&read[0]<='9')
        rewind(fp);
    while(!feof(fp)){//按组读取入data_array然后整体送入
        long tx_id;
        int ID;
        char from[35],to[35];
        double amount;
        fscanf(fp,"%ld,%d,%[^,],%lf,%[^\r~\n]\n",&tx_id,&ID,from,&amount,to);
        if(!data_length||data_array[data_length-1].ID==ID){
            if(data_length>=data_size){
                data_array = (data*)realloc(data_array,(data_size+50)*sizeof(data));
                if(!data_array)
                    exit(OVERFLOW);
                data_size += 50;
            }
            data_array[data_length].ID = ID;
            data_array[data_length].tx_id = tx_id;
            data_array[data_length].amount = amount;
            strcpy(data_array[data_length].from,from);
            strcpy(data_array[data_length].to,to);
            data_length++;
        }
        else {//在p节点上插入一个帐号的一组交易信息
            p = H.p[data_array[data_length-1].ID-80339];
            InsertBTree(p,data_array,data_length);
            L->time += data_length;
            data_array = (data*)realloc(data_array,50*sizeof(data));
            if(!data_array)
                exit(OVERFLOW);
            data_length = 0;
            data_size = 50;
            data_array[data_length].ID = ID;
            data_array[data_length].tx_id = tx_id;
            data_array[data_length].amount = amount;
            strcpy(data_array[data_length].from,from);
            strcpy(data_array[data_length].to,to);
            data_length++;
        }
    }
    p = L->next;
    while (data_array[data_length-1].ID>p->ID)
        p = p->next;
    if (!p)
        return ERROR;
    InsertBTree(p,data_array,data_length);//插入最后一组交易信息
    L->time += data_length;
    free(data_array);
    finish = clock();
    printf("Complete!\nInitialized Trade Data in %ldms\n",(finish-start));
    fclose(fp);
    return OK;
}
Status HashInit(Block_List &L,char *a,char *b,HashMap_B &H){//初始化
    H.length = 0;
    H.p = (Block_Node**)malloc(150000*sizeof(Block_Node*));
    if(!H.p)
        exit(OVERFLOW);
    memset(H.p,0,150000* sizeof(Block_Node*));
    InitBlockList(L,a,H);
    CreateTree(L,b,H);
    return OK;
}

Status AddData(Block_List &L,char *a,HashMap_B &H){//添加数据
    CreateTree(L,a,H);
    return OK;
}

Status DestroyData(Block_List &L,HashMap_B &H){//退出时销毁
    Block_Node *p = L->next;
    Block_Node *pp = p->next;
    while(p){
        if(p->from){
            vector v;
            InitVector(v);
            Push(v,p->from,0);
            while(v.length>0) {
                Tree_Node *tmp = v.p[v.length - 1];
                Trade_Node *tsp = tmp->pt;
                Trade_Node *tspp = tsp->nextfrom;
                while(tsp){//释放每个交易信息节点
                    free(tsp);
                    tsp = tspp;
                    if(tspp)
                        tspp = tspp->nextfrom;
                }
                Pop(v);
                if (tmp->rchild)
                    Push(v, tmp->rchild, 0);
                if (tmp->lchild)
                    Push(v, tmp->lchild, 0);
                free(tmp);
            }
            Push(v,p->to,0);
            while(v.length>0) {
                Tree_Node *tmp = v.p[v.length - 1];
                Pop(v);
                if (tmp->rchild)
                    Push(v, tmp->rchild, 0);
                if (tmp->lchild)
                    Push(v, tmp->lchild, 0);
                free(tmp);//释放每个树节点
            }
            DestroyVector(v);
        }
        free(p);
        p = pp;
        if(pp)
            pp = pp->next;
    }
    free(L);
    free(H.p);
    return OK;
}

int SearchTrade(Block_List L,long time_begin,long time_end,char *name,int mode,int k){
    Trade_List T;//所有符合条件的交易存入链表
    T = (Trade_Node*)malloc(sizeof(Trade_Node));
    if(!T)
        exit(OVERFLOW);
    Block_Node *p = L->next;
    if(!mode)
        T->nextto = NULL;
    else
        T->nextfrom = NULL;
    while(p->time<time_begin) {//找到起始时间
        p = p->next;
        if(!p)
            return 0;
    }
    int num = 0;
    int dt = 0;
    while(p&&p->time<=time_end){
        if(!mode){//入账
            Tree_Node *tmp = p->to;
            while(tmp) {//找到目标帐号
                int index = strcmp(name,tmp->name);
                if(!index)
                    break;
                else if(index>0)
                    tmp = tmp->rchild;
                else
                    tmp = tmp->lchild;
            }
            if(tmp){
                Trade_Node *t = tmp->pt;
                while(t){//接入新节点
                    Trade_Node *pos = T;
                    while(pos->nextto&&pos->nextto->amount>t->amount)
                        pos = pos->nextto;
                    Trade_Node *newnode;
                    newnode = (Trade_Node*)malloc(sizeof(Trade_Node));
                    if(!newnode)
                        exit(OVERFLOW);
                    *newnode = *t;
                    newnode->nextto = pos->nextto;
                    pos->nextto = newnode;
                    t = t->nextto;
                    num++;
                }
                dt = 1;
            }
        }
        else{
            Tree_Node *tmp = p->from;
            while(tmp) {
                int index = strcmp(name,tmp->name);
                if(!index)
                    break;
                else if(index>0)
                    tmp = tmp->rchild;
                else
                    tmp = tmp->lchild;
            }
            if(tmp){
                Trade_Node *t = tmp->pt;
                while(t){
                    Trade_Node *pos = T;
                    while(pos->nextfrom&&pos->nextfrom->amount>t->amount)
                        pos = pos->nextfrom;
                    Trade_Node *newnode;
                    newnode = (Trade_Node*)malloc(sizeof(Trade_Node));
                    if(!newnode)
                        exit(OVERFLOW);
                    *newnode = *t;
                    newnode->nextfrom = pos->nextfrom;
                    pos->nextfrom = newnode;
                    t = t->nextfrom;
                    num++;
                }
                dt = 1;
            }
        }
        p = p->next;
    }
    if(!dt){
        printf("No Such an Account\n");
        return ERROR;
    }
    if(!mode) {
        printf("Totally Find %d Records to Account %s\n", num, name);
        printf("The First %d Records of the Largest Amount are:\n",k);
        Trade_Node *t = T->nextto;
        for (int i=0;i<k&&i<num;++i){
            printf("Number:%d Tx_id:%ld From %s To %s Amount:%g\n",i+1,t->tx_id,t->from,t->to,t->amount);
            t = t->nextto;
        }
        Trade_Node *pp = T;
        Trade_Node *p = T->nextto;
        while(pp) {
            free(pp);
            pp = p;
            if(p)
                p = p->nextto;
        }
    }
    else{
        printf("Totally Find %d Records from Account %s\n", num, name);
        printf("The First %d Records of the Largest Amount are:\n",k);
        Trade_Node *t = T->nextfrom;
        for (int i=0;i<k&&i<num;++i){
            printf("Number:%d Tx_id:%ld From %s To %s Amount:%g\n",i+1,t->tx_id,t->from,t->to,t->amount);
            t = t->nextfrom;
        }
        Trade_Node *pp = T;
        Trade_Node *p = T->nextfrom;
        while(pp) {
            free(pp);
            pp = p;
            if(p)
                p = p->nextfrom;
        }
    }
    return num;
}
double MoneySta(Block_List L,long time,char *name){//算钱
    double amount = 0;
    int dt = 0;
    Block_Node *p = L->next;
    while(p&&p->time<=time){//目标时间以前
        Tree_Node *tmp = p->to;
        while(tmp) {//找到目标帐号
            int index = strcmp(name,tmp->name);
            if(!index)
                break;
            else if(index>0)
                tmp = tmp->rchild;
            else
                tmp = tmp->lchild;
        }
        if(tmp){//如果找到了
            Trade_Node *t = tmp->pt;
            while(t){
                amount += t->amount;
                t = t->nextto;
            }
            dt = 1;
        }
        tmp = p->from;
        while(tmp) {
            int index = strcmp(name,tmp->name);
            if(!index)
                break;
            else if(index>0)
                tmp = tmp->rchild;
            else
                tmp = tmp->lchild;
        }
        if(tmp){
            Trade_Node *t = tmp->pt;
            while(t){
                amount -= t->amount;
                t = t->nextfrom;
            }
            dt = 1;
        }
        p = p->next;
    }
    if(!dt){
        printf("No Such an Account\n");
        return ERROR;
    }
    printf("Money of this Account is %g\n",amount);
    return amount;
}

struct RankNode{//排行榜节点
    double amount;
    char name[35];
};

struct HashMap{//加快速度的哈希表
    RankNode **p;
    int *t;
    int length;
};

unsigned int BKDRHash(char *str,int l){//字符串哈希函数
    unsigned int seed = 31;// 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;
    while (*str)
        hash = hash*seed+(*str++);
    return ((hash&0x7FFFFFFF)%l);
}

HashMap *STH;//用于qsort的全局变量
int compare(const void *a,const void *b){
    return (int)((*STH).p[*(int*)b]->amount-(*STH).p[*(int*)a]->amount);
}

void quickSort(HashMap &H, int bgn, int end)  //arr must be the reference of real param
{
    if (bgn >= end - 1)//数组arr空or仅有一个元素则退出
        return;
    int lindex = bgn;
    int rindex = end - 1;
    int std = H.t[lindex];
    while (lindex < rindex){
        while (lindex < rindex){
            if (H.p[H.t[rindex]]->amount >= H.p[std]->amount){
                H.t[lindex++] = H.t[rindex];
                break;
            }
            --rindex;
        }
        while (lindex < rindex){
            if (H.p[H.t[lindex]]->amount < H.p[std]->amount){
                H.t[rindex--] = H.t[lindex];
                break;
            }
            ++lindex;
        }
    }
    H.t[lindex] = std;
    quickSort(H, bgn, lindex);
    quickSort(H, rindex + 1, end);
}

Status MoneyRank(Block_List L,long time,int k){
    clock_t start = clock();
    HashMap H;
    int times = 0;
    H.p = (RankNode**)malloc(1000000*sizeof(RankNode*));
    memset(H.p,0,1000000*sizeof(RankNode*));
    H.t = (int*)malloc(1000000*sizeof(int));
    H.length = 0;
    Block_Node *p = L->next;
    while(p&&p->time<=time) {//目标时间以前
        if(p->from){
            vector v;
            InitVector(v);
            Push(v,p->from,0);
            while(v.length>0) {
                Tree_Node *tmp = v.p[v.length - 1];
                unsigned int index = BKDRHash(tmp->name,1000000);
                if(H.p[index])
                    times++;//冲突次数计数
                while(H.p[index]&&strcmp(H.p[index]->name,tmp->name)){
                    index++;
                    if(index>=1000000)
                        index = 0;
                }//哈希函数寻找
                if(!H.p[index]) {//新帐号就加入
                    RankNode *newnode;
                    newnode = (RankNode *) malloc(sizeof(RankNode));
                    if (!newnode)
                        exit(OVERFLOW);
                    newnode->amount = 0;
                    strcpy(newnode->name, tmp->name);
                    H.p[index] = newnode;
                    Trade_Node *t = tmp->pt;
                    while (t) {
                        newnode->amount -= t->amount;
                        t = t->nextfrom;
                    }
                    H.t[H.length++] = index;
                }
                else{//更新钱数
                    Trade_Node *t = tmp->pt;
                    while (t) {
                        H.p[index]->amount -= t->amount;
                        t = t->nextfrom;
                    }
                }
                Pop(v);
                if (tmp->rchild)
                    Push(v, tmp->rchild, 0);
                if (tmp->lchild)
                    Push(v, tmp->lchild, 0);
            }
            Push(v,p->to,0);
            while(v.length>0) {//入节点
                Tree_Node *tmp = v.p[v.length - 1];
                unsigned int index = BKDRHash(tmp->name,1000000);
                while(H.p[index]&&strcmp(H.p[index]->name,tmp->name)){
                    index++;
                    if(index>=1000000)
                        index = 0;
                }
                if(!H.p[index]) {//如果没找到
                    RankNode *newnode;
                    newnode = (RankNode *) malloc(sizeof(RankNode));
                    if (!newnode)
                        exit(OVERFLOW);
                    newnode->amount = 0;
                    strcpy(newnode->name, tmp->name);
                    H.p[index] = newnode;
                    Trade_Node *t = tmp->pt;
                    while (t) {
                        newnode->amount += t->amount;
                        t = t->nextto;
                    }
                    H.t[H.length++] = index;
                }
                else{//找到了
                    Trade_Node *t = tmp->pt;
                    while (t) {
                        H.p[index]->amount += t->amount;
                        t = t->nextto;
                    }
                }
                Pop(v);
                if (tmp->rchild)
                    Push(v, tmp->rchild, 0);
                if (tmp->lchild)
                    Push(v, tmp->lchild, 0);
            }
            DestroyVector(v);
        }
        p = p->next;
    }
    STH = &H;
    qsort(H.t,H.length,4,compare);//排序
//    quickSort(H,0,H.length);
#if TEST
    clock_t finish = clock();
    printf("Complete in %ldms\n",(finish-start));
    printf("Conflict %d\n",times);
#endif
    printf("The Richest %d Account is:\n",k);
    for(int i=0;i<H.length&&i<k;++i)
        printf("%s Amount:%g\n",H.p[H.t[i]]->name,H.p[H.t[i]]->amount);
    free(H.p);
    free(H.t);
    return OK;
}

typedef struct GraphArc{//图节点
    double amount;
    GraphArc *nextin,*nextout;
    char namein[35],nameout[35];
}GraphArc;

typedef struct GraphVex{//图弧
    int in,out;
    GraphArc *firstin,*firstout;
    GraphVex *next;
    int check_in;
    char name[35];
}GraphVex,*Graph_List;

struct HashMap_G{//哈希表
    GraphVex **p;
    int *t;
    int length;
};

struct HaspMap_A{//哈希表
    GraphArc **p;
    int length;
};

Status CreatGra(Block_List L,Graph_List &G,HashMap_G &H){//创建图
    printf("Graph Initializing...\nLoading...\n");
    clock_t start = clock();
    HaspMap_A HA;
    HA.p = (GraphArc**)malloc(1500000* sizeof(GraphArc*));
    if(!HA.p)
        exit(OVERFLOW);
    memset(HA.p,0,1500000* sizeof(GraphArc*));
    HA.length = 0;
    H.p = (GraphVex**)malloc(1000000*sizeof(GraphVex*));
    H.t = (int*)malloc(1000000* sizeof(int));
    if(!H.p||!H.t)
        exit(OVERFLOW);
    memset(H.p,0,1000000*sizeof(GraphVex*));
    H.length = 0;
    G = (GraphVex*)malloc(sizeof(GraphVex));
    if(!G)
        exit(OVERFLOW);
    G->in = G->out = 0;
    G->firstin = G->firstout = NULL;
    G->next = NULL;
    GraphVex *np = G;
    Block_Node *p = L->next;
    while(p) {
        if(p->from) {
            vector v;
            InitVector(v);
            Push(v, p->from, 0);
            while (v.length > 0) {//对树节点上帐号新建图节点
                Tree_Node *tmp = v.p[v.length - 1];
                unsigned int index = BKDRHash(tmp->name,1000000);
                while (H.p[index] && strcmp(H.p[index]->name, tmp->name)) {
                    index++;
                    if (index >= 1000000)
                        index = 0;
                }
                GraphVex *newvex;
                if (!H.p[index]) {
                    newvex = (GraphVex *) malloc(sizeof(GraphVex));
                    if (!newvex)
                        exit(OVERFLOW);
                    newvex->in = newvex->out = 0;
                    newvex->firstout = newvex->firstin = NULL;
                    newvex->next = NULL;
                    strcpy(newvex->name, tmp->name);
                    H.p[index] = newvex;
                    H.t[H.length++] = index;
                    G->in++;
                    np->next = newvex;
                    np = np->next;
                }
                else
                    newvex = H.p[index];
                Trade_Node *t = tmp->pt;
                while (t) {//对树上链表各帐号进行处理
                    unsigned int Gindex = BKDRHash(t->to,1000000);
                    while (H.p[Gindex] && strcmp(H.p[Gindex]->name, t->to)) {
                        Gindex++;
                        if (Gindex >= 1000000)
                            Gindex = 0;
                    }
                    GraphVex *newGvex;
                    if (!H.p[Gindex]) {//没找到就新建
                        newGvex = (GraphVex *) malloc(sizeof(GraphVex));
                        if (!newGvex)
                            exit(OVERFLOW);
                        newGvex->in = newGvex->out = 0;
                        newGvex->firstout = newGvex->firstin = NULL;
                        newGvex->next = NULL;
                        strcpy(newGvex->name, t->to);
                        H.p[Gindex] = newGvex;
                        H.t[H.length++] = Gindex;
                        G->in++;
                        np->next = newGvex;
                        np = np->next;
                    }
                    else
                        newGvex = H.p[Gindex];
                    char comname[72];
                    strcpy(comname,t->from);
                    strcat(comname,t->to);//组合两个账号进行哈希，这样每一对交易都是哈希表上一个存储
                    unsigned int Aindex = BKDRHash(comname,1500000);//联合
                    while (HA.p[Aindex]&&(strcmp(HA.p[Aindex]->namein,t->to)||strcmp(HA.p[Aindex]->nameout,t->from))) {
                        Aindex++;
                        if (Aindex >= 1500000)
                            Aindex = 0;
                    }
                    if (!HA.p[Aindex]) {//没有就新建弧
                        GraphArc *newArc;
                        newArc = (GraphArc *) malloc(sizeof(GraphArc));
                        if (!newArc)
                            exit(OVERFLOW);
                        newArc->amount = t->amount;
                        strcpy(newArc->namein, t->to);
                        strcpy(newArc->nameout, t->from);
                        newArc->nextin = newGvex->firstin;
                        newArc->nextout = newvex->firstout;
                        HA.p[Aindex] = newArc;
                        newGvex->firstin = newArc;
                        newvex->firstout = newArc;
                        newvex->out++;
                        newGvex->in++;
                        G->out++;
                    }
                    else {//更新交易量
                        HA.p[Aindex]->amount += t->amount;
                        G->out++;
                    }
                    t = t->nextfrom;
                }
                Pop(v);
                if (tmp->rchild)
                    Push(v, tmp->rchild, 0);
                if (tmp->lchild)
                    Push(v, tmp->lchild, 0);
            }
            DestroyVector(v);
        }
        p = p->next;
    }
    free(HA.p);
    clock_t finish = clock();
    printf("Creating the Graph in %ldms\n",(finish-start));
    return OK;
}

HashMap_G *GTH;

int compare_Gin(const void*a,const void*b){
    return (*GTH).p[*(int*)b]->in-(*GTH).p[*(int*)a]->in;
}
int compare_Gout(const void*a,const void*b){
    return (*GTH).p[*(int*)b]->out-(*GTH).p[*(int*)a]->out;
}

Status StaGraph(Graph_List L,HashMap_G &H,int k,int mode){//入度出度
    int degree = 0;
    GraphVex *p = L->next;
    while(p){
        degree += p->in;
        p = p->next;
    }
    double A_degree;
    A_degree = (double)(degree)/L->in;
    printf("The Average In-Degree and Out-Degree is %g\n",A_degree);
    GTH = &H;
    if(!mode){
        qsort(H.t,H.length,4,compare_Gin);
        printf("The %d Accounts with Most In-Degree are:\n",k);
        for(int i=0;i<k;++i){
            printf("%s In-Degree:%d\n",H.p[H.t[i]]->name,H.p[H.t[i]]->in);
        }
    }
    else{
        qsort(H.t,H.length,4,compare_Gout);
        printf("The %d Accounts with Most Out-Degree are:\n",k);
        for(int i=0;i<k;++i){
            printf("%s Out-Degree:%d\n",H.p[H.t[i]]->name,H.p[H.t[i]]->out);
        }
    }
    return OK;
}

Status TopCheck(Graph_List &L,HashMap_G H){//拓扑排序
    int *stack;//手动实现一个栈用来优化时间性能
    stack = (int*)malloc(1000000*sizeof(int));
    if(!stack)
        exit(OVERFLOW);
    int length = 0;
    GraphVex *p = L->next;
    while(p){
        p->check_in = p->in;
        p = p->next;
    }
    int time = 0;
    for(int i=0;i<H.length;++i){//压入每一个入度为0的节点
        if(!H.p[H.t[i]]->check_in)
            stack[length++] = H.t[i];
    }
    while(time<H.length&&length>0) {
        int pos = stack[--length];
        time++;
        GraphArc *t = H.p[pos]->firstout;
        while (t) {
            unsigned index = BKDRHash(t->namein, 1000000);
            while (H.p[index] && strcmp(H.p[index]->name, t->namein)) {
                index++;
                if (index >= 1000000)
                    index = 0;
            }//寻找节点
            if (H.p[index]) {
                H.p[index]->check_in--;
                if(!H.p[index]->check_in)//如果处理之后该节点入度变为0
                    stack[length++] = index;
                else if(H.p[index]->check_in<0){//入度为负数的意外情况
                    printf("WTF");
                    exit(OVERFLOW);
                }
            }
            else{//没找到该节点的意外情况
                printf("??");
                exit(OVERFLOW);
            }
            t = t->nextout;
        }
    }
    if(time<H.length)
        printf("YES\n");
    else
        printf("NO\n");
    free(stack);
    return OK;
}

struct dijpair{//dijkstra排序中所用的数据结构
    double length;
    char name[35];
    bool operator <(const dijpair d) const{
        return this->length>d.length;//最小堆
    }
};

Status Dijkstra(Graph_List &L,HashMap_G H,char *name){
    GraphVex *t = L->next;
    while(t){
        t->check_in = 0;
        t = t->next;
    }
    std::priority_queue<dijpair> q;//堆优化
    unsigned int index = BKDRHash(name,1000000);
    while (H.p[index]&&strcmp(H.p[index]->name,name)) {
        index++;
        if (index >= 1000000)
            index = 0;
    }
    if(!H.p[index]){
        printf("No Such an Account!");
        return ERROR;
    }
    dijpair p;
    p.length = 0;
    strcpy(p.name,H.p[index]->name);
    q.push(p);
    while(!q.empty()){
        p = q.top();
        q.pop();
        index = BKDRHash(p.name,1000000);
        while (H.p[index]&&strcmp(H.p[index]->name,p.name)) {
            index++;
            if (index >= 1000000)
                index = 0;
        }
        if(H.p[index]->check_in)
            continue;
        H.p[index]->check_in = 1;
        GraphArc *ta = H.p[index]->firstout;
        while(ta){//松弛
            dijpair newp;
            newp.length = p.length+ta->amount;//更新当前长度
            strcpy(newp.name,ta->namein);
            q.push(newp);
            ta = ta->nextout;
        }
        printf("%s Path Length:%g\n",p.name,p.length);
    }
    return OK;
}

Status DestroyGraph(Graph_List &L,HashMap_G &H){
    free(H.p);
    free(H.t);
    GraphVex *t = L->next;
    while(t){
        GraphVex *tt = t->next;
        GraphArc *p = t->firstout;
        while(p){
            GraphArc *pp = p->nextout;
            free(p);
            p = pp;
        }
        free(t);
        t = tt;
    }
    free(L);
}

void PrintInfo(){
    printf("\n");
    printf("**************************************************************************\n");
    printf("Input 0 to Search Mode\n");
    printf("Input 1 to Analyse Mode\n");
    printf("Input 2 to Add Data\n");
    printf("Input -1 to Exit\n");
    printf("**************************************************************************\n");
}
void PrintInfoSearch(){
    printf("\n");
    printf("**************************************************************************\n");
    printf("Search Mode:\n");
    printf("Input 0 to Find the Trade Data of an Account over a Period of Time\n");
    printf("Input 1 to Find the Amount of an Account at a Given Moment\n");
    printf("Input 2 to Output the Rich Ranking List at a Given Moment\n");
    printf("Input -1 to Return\n");
    printf("**************************************************************************\n");
}

void PrintInfoAnalyse(){
    printf("\n");
    printf("**************************************************************************\n");
    printf("Analyse Mode:\n");
    printf("Input 0 to Find the Accounts with Most In-Degree Or Out-Degree\n");
    printf("Input 1 to Check If There Are Rings in the Graph\n");
    printf("Input 2 to Get the Shortest Path of an Account to Others\n");
    printf("Input -1 to Return\n");
    printf("**************************************************************************\n");
}
int main() {
    Block_List B_List;
    HashMap_B B_H;
    HashInit(B_List,"block_part1.csv","tx_data_part1_v2.csv",B_H);
    while(1) {
        PrintInfo();
        char instr[100];
        int select;
        scanf("%s",instr);
        select = strtol(instr,NULL,10);
        if(!select&&instr[0]!='0')
            select = -2;
        if (select==0){
            while (1) {
                PrintInfoSearch();
                char ninstr[100];
                int m;
                scanf("%s",ninstr);
                m = strtol(ninstr,NULL,10);
                if(!m&&ninstr[0]!='0')
                    m = -2;
                if(m==0){
                    char ainstr[100];
                    long time_begin, time_end;
                    int mode, k;
                    char name[35];
                    printf("\nInput the Begin time:\n");
                    scanf("%s",ainstr);
                    time_begin = strtol(ainstr,NULL,10);
                    if((!time_begin&&ainstr[0]!='0')||time_begin<0){
                        printf("Illegal Input!\n");
                        continue;
                    }
                    printf("Input the End time:\n");
                    scanf("%s",ainstr);
                    time_end = strtol(ainstr,NULL,10);
                    if((!time_end&&ainstr[0]!='0')||time_end<0||time_end<time_begin){
                        printf("Illegal Input!\n");
                        continue;
                    }
                    getchar();
                    printf("Input the Account:\n");
                    scanf("%s",name);
                    printf("Input 0 to Find Records to this Account Or 1 to Find Records from this Account:\n");
                    scanf("%s",ainstr);
                    mode = strtol(ainstr,NULL,10);
                    if((!mode&&ainstr[0]!='0')||(mode!=0&&mode!=1)){
                        printf("Illegal Input!\n");
                        continue;
                    }
                    printf("Input the Max Number of Records You Want to Output:\n");
                    scanf("%s",ainstr);
                    k = strtol(ainstr,NULL,10);
                    if((!k&&ainstr[0]!='0')||k<=0){
                        printf("Illegal Input!\n");
                        continue;
                    }
                    int num = SearchTrade(B_List, time_begin, time_end, name, mode, k);
                }
                else if(m==1){
                    char ainstr[100];
                    long time;
                    char name[35];
                    printf("\nInput the End Moment\n");
                    scanf("%s", ainstr);
                    time = strtol(ainstr,NULL,10);
                    if((!time&&ainstr[0]!='0')||time<0){
                        printf("Illegal Input!\n");
                        continue;
                    }
                    getchar();
                    printf("Input the Account:\n");
                    scanf("%s",name);
                    double amount = MoneySta(B_List, time, name);
                }
                else if(m==2){
                    char ainstr[100];
                    long time;
                    int k;
                    printf("\nInput the End Moment\n");
                    scanf("%s",ainstr);
                    time = strtol(ainstr,NULL,10);
                    if((!time&&ainstr[0]!='0')||time<0){
                        printf("Illegal Input!\n");
                        continue;
                    }
                    printf("Input the Max Number of Account You Want to Output:\n");
                    scanf("%s",ainstr);
                    k = strtol(ainstr,NULL,10);
                    if((!k&&ainstr[0]!='0')||k<=0){
                        printf("Illegal Input!\n");
                        continue;
                    }
                    MoneyRank(B_List,time,k);
                }
                else if(m==-1)
                    break;
                else
                    printf("Illegal Input!\n");
            }
        }
        else if (select==1) {
            Graph_List G_L;
            HashMap_G H;
            CreatGra(B_List,G_L,H);
            while (1) {
                PrintInfoAnalyse();
                char ninstr[100];
                int m;
                scanf("%s",ninstr);
                m = strtol(ninstr,NULL,10);
                if(!m&&ninstr[0]!='0')
                    m = -2;
                if(m==0){
                    printf("\nInput 0 to Check In-Degree Or 1 to Check Out-Degree\n");
                    char ainstr[100];
                    int k,mode;
                    scanf("%s",ainstr);
                    mode = strtol(ainstr,NULL,10);
                    if((!mode&&ainstr[0]!='0')||(mode!=0&&mode!=1)){
                        printf("Illegal Input!\n");
                        continue;
                    }
                    printf("Input the Max Number of Account You Want to Output:\n");
                    scanf("%s",ainstr);
                    k = strtol(ainstr,NULL,10);
                    if((!k&&ainstr[0]!='0')||k<=0){
                        printf("Illegal Input!\n");
                        continue;
                    }
                    StaGraph(G_L,H,k,mode);
                }
                else if(m==1){
                    TopCheck(G_L,H);
                }
                else if(m==2){
                    printf("Input the Account:\n");
                    char name[35];
                    scanf("%s",name);
                    Dijkstra(G_L,H,name);
                }
                else if(m==-1){
                    DestroyGraph(G_L,H);
                    break;
                }
                else
                    printf("Illegal Input!\n");
            }
        }
        else if(select==2){
            char url[150];
            printf("Input the File Path:\n");
            scanf("%s",url);
            AddData(B_List,url,B_H);
        }
        else if (select==-1) {
            DestroyData(B_List,B_H);
            return 0;
        }
        else
            printf("Illegal Input!\n");
    }
}