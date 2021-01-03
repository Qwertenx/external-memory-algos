#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include <queue>
template<typename TKey>
class BTreeSet
{
private:
	bool binary;
	int parameter;
	int rootIndex;
	int lastIndex;
	std::string storageLocation;
	struct Node
	{
		int size;
		bool leaf;
		int index;
		TKey* keys = nullptr;
		int* children = nullptr;
		~Node()
		{
			if (keys != nullptr)
				delete[] keys;
			if (children != nullptr)
				delete[] children;
		}
	};
	std::string getTempFileName(const std::string& storageLocation, int index) const;
	void readNode(std::shared_ptr<Node> &node, int index) const;
	void writeNode(std::shared_ptr<Node> node);
	void createNode(std::shared_ptr<Node> &node);
	void insert(std::shared_ptr<Node> node, const TKey& key);
	void removeFromLeaf(std::shared_ptr<Node> leaf, int keyIndex, bool rootCall);
	void removeFromInternal(std::shared_ptr<Node> node, int keyIndex, bool rootCall);
	TKey removeMin(std::shared_ptr<Node> node);
	void erase(std::shared_ptr<Node> node, const TKey& key);
	bool find(std::shared_ptr<Node> node, const TKey& key) const;
	std::shared_ptr<Node> split(std::shared_ptr<Node> parent, std::shared_ptr<Node> childPtr, int child);
	void merge(std::shared_ptr<Node> parent, std::shared_ptr<Node> firstPart, std::shared_ptr<Node> secondPart, int firstPartIndex, bool direction);
	void keyRepositioning(std::shared_ptr<Node> parent, std::shared_ptr<Node> firstPart, std::shared_ptr<Node> secondPart, int firstPartIndex, bool direction);
	bool eraseResolve(std::shared_ptr<Node> parent, std::shared_ptr<Node> child, int childIndex);
	void backupTree() const;
public:
	BTreeSet(int parameter, bool binary, std::string storageLocation);
	void insert(TKey key);
	void erase(TKey key);
	bool find(TKey key) const;
	~BTreeSet();
};

template<typename TKey>
BTreeSet<TKey>::BTreeSet(int parameter, bool binary, std::string storageLocation)
{
	FILE* input;
	fopen_s(&input, (storageLocation + "/treeParams.bin").c_str(), "rb");
	if (input)
	{
		fread(&this->parameter, sizeof(int), 1, input);
		fread(&rootIndex, sizeof(int), 1, input);
		fread(&lastIndex, sizeof(int), 1, input);
		fread(&this->binary, sizeof(bool), 1, input);
		this->storageLocation = storageLocation;
		fclose(input);
	}
	else
	{
		lastIndex = 0;
		rootIndex = 0;
		this->parameter = parameter;
		this->storageLocation = storageLocation;
		this->binary = binary;
		std::shared_ptr<Node> root = nullptr;
		createNode(root);
		writeNode(root);
		backupTree();
	}
}

template<typename TKey>
BTreeSet<TKey>::~BTreeSet()
{
	backupTree();
}

template<typename TKey>
std::string BTreeSet<TKey>::getTempFileName(const std::string& storageLocation, int index) const
{
	return (storageLocation + "/node" + std::to_string(index) + ".bin");
}

template<typename TKey>
void BTreeSet<TKey>::backupTree() const
{
	FILE* output;
	fopen_s(&output, (storageLocation + "/treeParams.bin").c_str(), "wb");
	fwrite(&parameter, sizeof(int), 1, output);
	fwrite(&rootIndex, sizeof(int), 1, output);
	fwrite(&lastIndex, sizeof(int), 1, output);
	fwrite(&binary, sizeof(bool), 1, output);
	fclose(output);
}

template<typename TKey>
void BTreeSet<TKey>::readNode(std::shared_ptr<Node> &node, int index) const
{
	node = std::shared_ptr<Node>(new Node());
	if (binary)
	{
		FILE* fileIn;
		if (int x = fopen_s(&fileIn, getTempFileName(storageLocation, index).c_str(), "rb"))
			return;
		node->keys = new TKey[2 * parameter - 1];
		node->index = index;
		fread(&node->size, sizeof(int), 1, fileIn);
		fread(&node->leaf, sizeof(bool), 1, fileIn);
		fread(node->keys, sizeof(TKey), node->size, fileIn);
		if (!node->leaf)
		{
			node->children = new int[2 * parameter];
			fread(node->children, sizeof(int), node->size + 1, fileIn);
		}
		fclose(fileIn);
	}
	else
	{
		std::ifstream fileIn(getTempFileName(storageLocation, index));
		fileIn >> node->size;
		fileIn >> node->leaf;
		node->index = index;
		node->keys = new TKey[2 * parameter - 1];
		for (int i = 0; i < node->size; i++)
			fileIn >> node->keys[i];
		if (!node->leaf)
		{
			node->children = new int[2 * parameter];
			for (int i = 0; i < node->size + 1; i++)
				fileIn >> node->children[i];
		}
		fileIn.close();
	}
}

template<typename TKey>
void BTreeSet<TKey>::writeNode(std::shared_ptr<Node> node)
{
	if (node->size == 0 && node->index != rootIndex)
	{
		remove(getTempFileName(storageLocation, node->index).c_str());
		return;
	}
	if (binary)
	{
		FILE* fileOut;
		fopen_s(&fileOut, getTempFileName(storageLocation, node->index).c_str(), "wb");
		fwrite(&node->size, sizeof(int), 1, fileOut);
		fwrite(&node->leaf, sizeof(bool), 1, fileOut);
		fwrite(node->keys, sizeof(TKey), node->size, fileOut);
		if (!node->leaf)
			fwrite(node->children, sizeof(int), node->size + 1, fileOut);
		fclose(fileOut);
	}
	else
	{
		std::ofstream fileOut(getTempFileName(storageLocation, node->index));
		fileOut << node->size << " ";
		fileOut << node->leaf << " ";
		for (int i = 0; i < node->size; i++)
			fileOut << node->keys[i] << " ";
		for (int i = 0; i < node->size; i++)
			fileOut << node->values[i] << " ";
		if (!node->leaf)
		{
			for (int i = 0; i < node->size + 1; i++)
				fileOut << node->children[i] << " ";
		}
		fileOut.close();
	}
}

template<typename TKey>
void BTreeSet<TKey>::createNode(std::shared_ptr<Node> &node)
{
	node = std::shared_ptr<Node>(new Node());
	node->size = 0;
	node->index = lastIndex++;
	node->leaf = true;
	node->keys = new TKey[2 * parameter - 1];
	node->children = new int[2 * parameter];
}

template<typename TKey>
std::shared_ptr<typename BTreeSet<TKey>::Node> BTreeSet<TKey>::split(std::shared_ptr<Node> parent, std::shared_ptr<Node> childPtr, int child)
{
	std::shared_ptr<Node> firstPart, secondPart = nullptr;
	firstPart = childPtr;
	createNode(secondPart);
	secondPart->leaf = firstPart->leaf;
	for (int keyIndex = 0; keyIndex < parameter - 1; keyIndex++)
	{
		secondPart->keys[keyIndex] = firstPart->keys[keyIndex + parameter];
		secondPart->values[keyIndex] = firstPart->values[keyIndex + parameter];
	}
	if (!firstPart->leaf)
		for (int childIndex = 0; childIndex < parameter; childIndex++)
			secondPart->children[childIndex] = firstPart->children[childIndex + parameter];
	firstPart->size = parameter - 1;
	secondPart->size = parameter - 1;
	for (int keyIndex = parent->size; keyIndex > child; keyIndex--)
	{
		parent->keys[keyIndex] = parent->keys[keyIndex - 1];
		parent->values[keyIndex] = parent->values[keyIndex - 1];
	}
	parent->keys[child] = firstPart->keys[parameter - 1];
	parent->values[child] = firstPart->values[parameter - 1];
	for (int childIndex = parent->size + 1; childIndex > child + 1; childIndex--)
		parent->children[childIndex] = parent->children[childIndex - 1];
	parent->children[child + 1] = secondPart->index;
	parent->size++;
	writeNode(parent);
	writeNode(firstPart);
	writeNode(secondPart);
	return secondPart;
}

template<typename TKey>
void BTreeSet<TKey>::merge(std::shared_ptr<Node> parent, std::shared_ptr<Node> firstPart, std::shared_ptr<Node> secondPart, int firstPartIndex, bool direction)
{
	if (direction)
	{
		firstPart->keys[firstPart->size] = parent->keys[firstPartIndex];
		firstPart->values[firstPart->size] = parent->values[firstPartIndex];
		for (int i = 0; i < secondPart->size; i++)
		{
			firstPart->keys[firstPart->size + i + 1] = secondPart->keys[i];
			firstPart->values[firstPart->size + i + 1] = secondPart->values[i];
			if (!firstPart->leaf)
				firstPart->children[firstPart->size + i + 1] = secondPart->children[i];
		}
		if (!firstPart->leaf)
			firstPart->children[firstPart->size + secondPart->size + 1] = secondPart->children[secondPart->size];
		for (int i = firstPartIndex; i < parent->size - 1; i++)
		{
			parent->keys[i] = parent->keys[i + 1];
			parent->values[i] = parent->values[i + 1];
			parent->children[i + 1] = parent->children[i + 2];
		}
	}
	else
	{
		for (int i = 0; i < firstPart->size; i++)
		{
			firstPart->keys[i + secondPart->size + 1] = firstPart->keys[i];
			firstPart->values[i + secondPart->size + 1] = firstPart->values[i];
			if (!firstPart->leaf)
				firstPart->children[i + secondPart->size + 1] = firstPart->children[i];
		}
		if (!firstPart->leaf)
			firstPart->children[firstPart->size + secondPart->size + 1] = firstPart->children[firstPart->size];
		for (int i = 0; i < secondPart->size; i++)
		{
			firstPart->keys[i] = secondPart->keys[i];
			firstPart->values[i] = secondPart->values[i];
			if (!firstPart->leaf)
				firstPart->children[i] = secondPart->children[i];
		}
		firstPart->keys[firstPart->size] = parent->keys[firstPartIndex - 1];
		firstPart->values[firstPart->size] = parent->values[firstPartIndex - 1];
		if (!firstPart->leaf)
			firstPart->children[firstPart->size] = secondPart->children[secondPart->size];
		for (int i = firstPartIndex - 1; i < parent->size - 1; i++)
		{
			parent->keys[i] = parent->keys[i + 1];
			parent->values[i] = parent->values[i + 1];
			parent->children[i] = parent->children[i + 1];
		}
		parent->children[parent->size - 1] = parent->children[parent->size];
	}
	firstPart->size = firstPart->size + secondPart->size + 1;
	parent->size--;
	secondPart->size = 0;
	writeNode(parent);
	writeNode(firstPart);
	writeNode(secondPart);
}

template<typename TKey>
void BTreeSet<TKey>::keyRepositioning(std::shared_ptr<Node> parent, std::shared_ptr<Node> firstPart, std::shared_ptr<Node> secondPart, int firstPartIndex, bool direction)
{
	if (direction)
	{
		firstPart->keys[firstPart->size] = parent->keys[firstPartIndex];
		firstPart->values[firstPart->size] = parent->values[firstPartIndex];
		if (!firstPart->leaf)
			firstPart->children[firstPart->size + 1] = secondPart->children[0];
		firstPart->size++;
		parent->keys[firstPartIndex] = secondPart->keys[0];
		parent->values[firstPartIndex] = secondPart->values[0];
		for (int i = 1; i < secondPart->size; i++)
		{
			secondPart->keys[i - 1] = secondPart->keys[i];
			secondPart->values[i - 1] = secondPart->values[i];
			if (!firstPart->leaf)
				secondPart->children[i - 1] = secondPart->children[i];
		}
		if (!firstPart->leaf)
			secondPart->children[secondPart->size - 1] = secondPart->children[secondPart->size];
		secondPart->size--;
	}
	else
	{
		for (int i = firstPart->size; i >= 1; i--)
		{
			firstPart->keys[i] = firstPart->keys[i - 1];
			firstPart->values[i] = firstPart->values[i - 1];
			if (!firstPart->leaf)
				firstPart->children[i + 1] = firstPart->children[i];
		}
		if (!firstPart->leaf)
		{
			firstPart->children[1] = firstPart->children[0];
			firstPart->children[0] = secondPart->children[secondPart->size];
		}
		firstPart->keys[0] = parent->keys[firstPartIndex - 1];
		firstPart->values[0] = parent->values[firstPartIndex - 1];
		firstPart->size++;
		parent->keys[firstPartIndex - 1] = secondPart->keys[secondPart->size - 1];
		parent->values[firstPartIndex - 1] = secondPart->values[secondPart->size - 1];
		secondPart->size--;
	}
	writeNode(parent);
	writeNode(firstPart);
	writeNode(secondPart);
}

template<typename TKey>
bool BTreeSet<TKey>::eraseResolve(std::shared_ptr<Node> parent, std::shared_ptr<Node> child, int childIndex)
{
	std::shared_ptr<Node> predecessor = nullptr, successor = nullptr;
	if (childIndex)
	{
		readNode(predecessor, parent->children[childIndex - 1]);
		if (predecessor->size != parameter - 1)
		{
			keyRepositioning(parent, child, predecessor, childIndex, 0);
			return true;
		}
	}
	if (childIndex != parent->size)
	{
		readNode(successor, parent->children[childIndex + 1]);
		if (successor->size != parameter - 1)
		{
			keyRepositioning(parent, child, successor, childIndex, 1);
			return false;
		}
	}
	bool result;
	if (predecessor != nullptr)
	{
		merge(parent, child, predecessor, childIndex, 0);
		result = true;
	}
	else if (successor != nullptr)
	{
		merge(parent, child, successor, childIndex, 1);
		result = false;
	}
	return result;
}

template<typename TKey>
void BTreeSet<TKey>::removeFromLeaf(std::shared_ptr<Node> leaf, int keyIndex, bool rootCall)
{
	for (int i = keyIndex; i < leaf->size - 1; i++)
	{
		leaf->keys[i] = leaf->keys[i + 1];
		leaf->values[i] = leaf->values[i + 1];
	}
	leaf->size--;

	writeNode(leaf);
}

template<typename TKey>
void BTreeSet<TKey>::removeFromInternal(std::shared_ptr<Node> node, int keyIndex, bool rootCall)
{
	TKey key = node->keys[keyIndex];
	std::shared_ptr<Node> child;
	readNode(child, node->children[keyIndex + 1]);
	if (child->size == parameter - 1)
		if (eraseResolve(node, child, keyIndex + 1))
		{
			if (rootCall)
			{
				if (node->size == 0)
				{
					rootIndex = node->children[0];
					writeNode(node);
				}
			}
			erase(child, key);
			return;
		}
	TKey result = removeMin(child);
	node->keys[keyIndex] = result;
	if (rootCall)
	{
		if (node->size == 0)
		{
			rootIndex = node->children[0];
		}
	}
	writeNode(node);
}

template<typename TKey>
TKey BTreeSet<TKey>::removeMin(std::shared_ptr<Node> node)
{
	if (node->leaf)
	{
		TKey result = node->keys[0];
		removeFromLeaf(node, 0, false);
		return result;
	}
	else
	{
		std::shared_ptr<Node> child;
		readNode(child, node->children[0]);
		if (child->size == parameter - 1)
			eraseResolve(node, child, 0);
		return removeMin(child);
	}
}

template<typename TKey>
bool BTreeSet<TKey>::find(std::shared_ptr<Node> node, const TKey& key) const
{
	int keyIndex = std::upper_bound(node->keys, node->keys + node->size, key) - node->keys - 1;
	if (keyIndex != -1 && node->keys[keyIndex] == key)
		return true;
	if (node->leaf)
		return false;
	std::shared_ptr<Node> child;
	readNode(child, node->children[keyIndex + 1]);
	return find(child, key);
}

template<typename TKey>
void BTreeSet<TKey>::insert(std::shared_ptr<Node> node, const TKey& key)
{
	int keyIndex;
	if (node->leaf)
	{
		for (keyIndex = node->size - 1; keyIndex >= 0 && key < node->keys[keyIndex]; keyIndex--)
		{
			node->keys[keyIndex + 1] = node->keys[keyIndex];
			node->values[keyIndex + 1] = node->values[keyIndex];
		}
		node->keys[keyIndex + 1] = key;
		node->size++;
		writeNode(node);
	}
	else
	{
		keyIndex = std::upper_bound(node->keys, node->keys + node->size, key) - node->keys - 1;
		int childIndex = keyIndex + 1;
		std::shared_ptr<Node> child;
		readNode(child, node->children[childIndex]);
		if (child->size == 2 * parameter - 1)
		{
			std::shared_ptr<Node> secondChild = split(node, child, childIndex);
			if (key < node->keys[childIndex])
			{
				insert(child, key);
			}
			else
			{
				insert(secondChild, key);
			}

		}
		else
		{
			insert(child, key);
		}
	}
}

template<typename TKey>
void BTreeSet<TKey>::erase(std::shared_ptr<Node> node, const TKey& key)
{
	int keyIndex;
	keyIndex = std::upper_bound(node->keys, node->keys + node->size, key) - node->keys - 1;
	if (keyIndex != -1 && node->keys[keyIndex] == key)
	{

		if (node->leaf)
			removeFromLeaf(node, keyIndex, false);
		else
			removeFromInternal(node, keyIndex, false);
	}
	else if (!node->leaf)
	{
		std::shared_ptr<Node> child;
		readNode(child, node->children[keyIndex + 1]);
		if (child->size == parameter - 1)
			eraseResolve(node, child, keyIndex + 1);
		erase(child, key);
	}
}

template<typename TKey>
bool BTreeSet<TKey>::find(TKey key) const
{
	std::shared_ptr<Node> root;
	readNode(root, rootIndex);
	return find(root, key);
}

template<typename TKey>
void BTreeSet<TKey>::insert(TKey key)
{
	std::shared_ptr<Node> root;
	readNode(root, rootIndex);
	if (root->size == 2 * parameter - 1)
	{
		std::shared_ptr<Node> newRoot = nullptr;
		createNode(newRoot);
		rootIndex = newRoot->index;
		newRoot->leaf = false;
		newRoot->children[0] = root->index;
		std::shared_ptr<Node> secondChild = split(newRoot, root, 0);
		insert(newRoot, key);
	}
	else
		insert(root, key);
	backupTree();
}

template<typename TKey>
void BTreeSet<TKey>::erase(TKey key)
{
	std::shared_ptr<Node> root;
	readNode(root, rootIndex);
	int keyIndex;
	keyIndex = std::upper_bound(root->keys, root->keys + root->size, key) - root->keys - 1;
	if (keyIndex != -1 && root->keys[keyIndex] == key)
	{
		if (root->leaf)
		{
			removeFromLeaf(root, keyIndex, true);
		}
		else
		{
			removeFromInternal(root, keyIndex, true);
		}
	}
	else if (!root->leaf)
	{
		std::shared_ptr<Node> child;
		readNode(child, root->children[keyIndex + 1]);
		if (child->size == parameter - 1)
		{
			eraseResolve(root, child, keyIndex + 1);
			if (root->size == 0)
			{
				rootIndex = root->children[0];
				writeNode(root);
			}
		}
		erase(child, key);
	}
	backupTree();
}