#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>
#include <string>


#define STORE_FILE "store/dumpFile"

std::mutex mtx;
std::string delimiter = ":";

// 节点类
template<typename K, typename V>
class Node{

public:
	Node() {}

	Node(K k, V v, int);

	~Node();

	K get_key() const;

	V get_value() const;

	void set_value(V);

	Node<K, V> **forward;

	int node_level;

private:
	K key;
	V value;
};

template<typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level) {
	this->key = k;
	this->value = v;
	this->node_level = level;

	this->forward = new Node<K, V>*[level+1];

	memset(this->forward, 0, sizeof(Node<K, V>*)*(level+1));
};

template<typename K, typename V>
Node<K, V>::~Node() {
	delete []forward;
};

template<typename K, typename V> 
K Node<K, V> ::get_key() const {
	return key;
};

template<typename K, typename V> 
V Node<K, V>::get_value() const {
	return value;
};

template<typename K, typename V> 
void Node<K, V>::set_value(V value) {
	this->value = value;
};


template <typename K, typename V>
class SkipList {
public:
	SkipList(int);
	~SkipList();
	int get_random_level();
	Node<K, V>* create_node(K, V, int);
	int insert_element(K, V);
	void display_list();
	bool search_element(K);
	void delete_element(K);
	void dump_file();
	void load_file();
	int size();

private:
	void get_key_value_from_string(const std::string& str, std::string* key, std::string* value);
	bool is_valid_string(const std::string& str);

private:
	int _max_level = 0;
	int _skip_list_level;

	Node<K, V> *_header;

    std::ofstream _file_writer;
    std::ifstream _file_reader;

	int _element_count;
};

template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(const K k, const V v, int level) {
	Node<K, V> *n = new Node<K, V>(k, v, level);
	return n;
}

// 插入
template<typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value) {
	mtx.lock();
	Node<K, V> *current = this->_header;

	Node<K, V>** update;
	update = new Node<K, V>* [_max_level + 1];
	memset(update, 0, sizeof(Node<K, V>*)*(_max_level+1));

// 每个节点有一个长度为level+1的Node数组arr
// arr[level]存放的是当前节点Node指针，arr[level-1]存放的是向下一个level的Node指针

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
	//updata = {40,30,30,10,1}

	// 指向 下一个Node
	current = current->forward[0];

	// 如果key已经存在
	if (current != NULL && current->get_key() == key) {
		std::cout << "key: " << key << ", exists" << std::endl;
		mtx.unlock();
		return 1;
	}

	// 如果current == NULL 说明我们到达了尾部
	// 如果current的key不相等说明找到了插入位置
	if (current == NULL || current->get_key() != key) {
		
		// 为节点生成随机level
		int random_level = get_random_level();

		// 如果随机level大于当前跳表level，更新update数组
		if (random_level > _skip_list_level) {
			for (int i = _skip_list_level+1; i < random_level+1; i++) {
				update[i] = _header;
			}
			_skip_list_level = random_level;
		}

		// 为生产的随机level创建一个新节点
		Node<K, V>* inserted_node = create_node(key, value, random_level);


		// 插入节点 *妙
		for (int i = 0; i <= random_level; i++) {
			inserted_node->forward[i] = update[i]->forward[i];
			update[i]->forward[i] = inserted_node;
		}
		std::cout << "Successfully inserted key:" << key << ", value:" << value << std::endl;
		_element_count ++;
	}
	delete[] update;
	mtx.unlock();
	return 0;
}

// 打印跳表
template<typename K, typename V>
void SkipList<K, V>::display_list() {

	std::cout << "\n*****Skip List*****" <<"\n";
	for (int i = 0; i <= _skip_list_level; i++) {
		Node<K, V> *node = this->_header->forward[i];
		std::cout << "Level" << i << ":";
		while (node != NULL) {
			std::cout << node->get_key() << ":" << node->get_value() << ";";
			node = node->forward[i];
		}
		std::cout << std::endl;
	}
}

// 写不进去，不知道为什么
// 将内存中的数据转储到文件
template<typename K, typename V>
void SkipList<K, V>::dump_file() {

    std::cout << "dump_file-----------------" << std::endl;
    _file_writer.open(STORE_FILE, std::ofstream::out| std::ofstream::trunc);
    Node<K, V> *node = this->_header->forward[0];

    while (node != NULL) {
    	_file_writer << node->get_key() << ":" << node ->get_value() << "\n";
    	std::cout << node->get_key() << ":" << node ->get_value() << "\n";
    	node = node->forward[0];
    }

    _file_writer.flush();
    _file_writer.close();
    return;
}

// 功能未测试
// 录入数据
template<typename K, typename V>
void SkipList<K, V>::load_file() {

	_file_reader.open(STORE_FILE);
    std::cout << "load_file-----------------" << std::endl;
    std::string line;
    std::string* key = new std::string();
    std::string* value = new std::string();
    while (getline(_file_reader, line)) {
    	get_key_value_from_string(line, key, value);
    	if(key->empty() || value->empty()) {
    		continue;
    	}
    	insert_element(*key, *value);
    	std::cout << "key:" << *key << "value:" << *value << std::endl;
    }
    _file_reader.close();
}

// 获得跳表大小
template<typename K, typename V>
int SkipList<K, V>::size() {
	return _element_count;
}

template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string& str, std::string* key, std::string* value) {
	if(!is_valid_string(str)) {
		return;
	}
	*key = str.substr(0, str.find(delimiter));
	*value = str.substr(str.find(delimiter)+1, str.length());
}

template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string& str) {
	if (str.empty()) {
		return false;
	}
	if (str.find(delimiter) == std::string::npos) {
		return false;
	}
	return true;
}

// 删除元素
template<typename K, typename V>
void SkipList<K, V>::delete_element(K key) {

	mtx.lock();
	Node<K, V> *current = this->_header;
	Node<K, V>** update;
	update = new Node<K, V>* [_max_level + 1];
	memset(update, 0, sizeof(Node<K, V>*) * (_max_level + 1));

	// 从跳表内最高的level开始
	for (int i = _skip_list_level; i >= 0; i--) {
		while (current->forward[i] != NULL && current->forward[i]->get_key() < key) {
			current = current->forward[i];
		}
		update[i] = current;
	}

	// 找到目标元素 && 删除操作
	current = current->forward[0];
	if (current != NULL && current->get_key() == key) {

		for (int i = 0; i <=_skip_list_level; i++) {
			// 如果下一个元素不是目标元素则跳出循环
			if (update[i]->forward[i] != current)
				break;

			update[i]->forward[i] = current->forward[i];
		}

		// 如果当前level没有元素则删除该level
		while (_skip_list_level > 0 && _header->forward[_skip_list_level] == 0) {
			_skip_list_level --;
		}
		
		std::cout << "Successfully deleted key "<< key << std::endl;
        _element_count --;
	}
	delete[] update;
	mtx.unlock();
	return;
}

// 查找
template<typename K, typename V>
bool SkipList<K, V>::search_element(K key) {

    std::cout << "search_element-----------------" << std::endl;
    Node<K, V> *current = _header;

    for (int i = _skip_list_level; i >= 0; i--) {
    	while (current->forward[i] && current->forward[i]->get_key() < key) {
    		current = current->forward[i];
    	}
    }

    current = current->forward[0];

    if (current != NULL && current->get_key() == key) {
    	std::cout << "Found key:" << key << ", value: " << current->get_value() << std::endl;
    	return true;
    }

    std::cout << "Not Found Key:" << key << std::endl;
    return false;
}

// 构造函数
template<typename K, typename V>
SkipList<K, V>::SkipList(int max_level) {
	this->_max_level = max_level;
	this->_skip_list_level = 0;
	this->_element_count = 0;

	// 创建头节点和初始化key、value为null
	K k{};
	V v{};
	this->_header = new Node<K, V>(k, v, _max_level);
};

template<typename K, typename V>
SkipList<K, V>::~SkipList() {

	if (_file_writer.is_open()) {
		_file_writer.close();
	}
	if (_file_reader.is_open()) {
		_file_reader.close();
	}
	delete _header;
}

template<typename K, typename V>
int SkipList<K, V>::get_random_level() {

	int k = 1;
	while (rand() % 2) {
		k++;
	}
	k = (k < _max_level) ? k : _max_level;
	return k;
};