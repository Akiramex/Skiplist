# 项目简述

相信只要是搞后端的同学应该都要熟悉非关系型数据库redis吧，那么应该知道redis的存储引擎是跳表实现的。

现在很多云厂商提供的云数据库，其底层都是用了Facebook开源的rocksdb，而rocksdb的底层是Google开源的Levedb，而Levedb的核心实现也是跳表。

本项目就是基于跳表实现的轻量级键值型存储引擎，使用C++实现。插入数据、删除数据、查询数据、数据展示、数据落盘、文件加载数据，以及数据库大小显示。

## 跳表

跳表全称为跳跃列表，它允许快速查询，插入和删除一个有序连续元素的数据链表。跳跃列表的平均查找和插入时间复杂度都是O(logn)。快速查询是通过维护一个多层次的链表，且每一层链表中的元素是前一层链表元素的子集（见右边的示意图）。一开始时，算法在最稀疏的层次进行搜索，直至需要查找的元素在该层两个相邻的元素中间。这时，算法将跳转到下一个层次，重复刚才的搜索，直到找到需要查找的元素为止。

## 函数

	int insert_element(K, V);	插入元素
	void display_list();		打印表格
	bool search_element(K);		查找元素
	void delete_element(K);		删除元素
	void dump_file();			写入数据
	void load_file();			加载数据
	int size();					返回大小



### 主要实现

1. 使用了C++模板编程

2. 节点设计：每个节点都有一个Node指针数组，每个节点有一个长度为level+1的Node数组arr
arr[level]存放的是当前节点Node指针，arr[level-1]存放的是向下一个level的Node指针

3.查找设计： 

~~~c++
/*                         +------------+
                           |  insert 50 |
                           +------------+
level 4     +-->1+                                         100
                 |
                 |                        
level 3         1+-------->10+				           70  100
                             |                 
                             |                 
level 2         1          10+-------->30+             70  100
                                    	 |     
                                         |     
level 1         1    4     10          30+             70  100
                                         |     
                                         |     					
level 0         1    4   9 10          30+---->40  60  70  100                                          
*/

	for(int i = _skip_list_level; i >= 0; i--) {
		while(current->forward[i] != NULL && current->forward[i]->get_key() < key) {
			current = current->forward[i];
		}
		update[i] = current; 
	}
~~~

## 问题

编译时遇到的问题：

1. update数组创建失败，报错提示：表达式必须含有常量值。解决方法：将update开辟到堆区，不再报错
2. 创建模板类的模板成员时候，报错提示：不能使用未初始化的成员。解决方法：使用零初始化 K k{}; V v{};
3. dump_file功能无法正确写入到文件中，排查不出问题。



## 声明

项目源自https://github.com/youngyangyang04/Skiplist-CPP
