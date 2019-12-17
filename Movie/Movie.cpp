﻿#include "BalancedBinaryTree.h"
#include "CharStringLink.h"
#include <iostream>
#include <fstream>
#include <string>
#include <io.h>
#include <windows.h>

using namespace std;

string Utf8ToGbk(const char *src_str)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
	wchar_t* wszGBK = new wchar_t[len + 1];
	memset(wszGBK, 0, (len + 1) * sizeof(wchar_t));

	MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);

	char* szGBK = new char[len + 1];
	memset(szGBK, 0, (len + 1) * sizeof(char));
	WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
	string strTemp(szGBK);

	if (wszGBK != NULL)
		delete[] wszGBK;
	if (szGBK != NULL)
		delete[] szGBK;

	return strTemp;
}

inline int CharToInt(char* c)
{
	int result = 0;
	int i = 0;
	while (!(c[i] >= '0' && c[i] <= '9'))
		i++;
	while (c[i] >= '0' && c[i] <= '9')
	{
		result = result * 10 + c[i] - '0';
		i++;
	}
	return result;
}

void GetFileList(CharStringLink* inFileList, const char* inPath, const char* inFormat)
{
	char PathFormat[100];
	strcpy(PathFormat, inPath);
	strcat(PathFormat, "*");
	strcat(PathFormat, inFormat);

	struct _finddata_t fileinfo;
	intptr_t handle;
	if ((handle = _findfirst(PathFormat, &fileinfo)) == -1)
	{
		cout << "No file found!" << endl;
		system("pause");
		exit(0);
	}
	else
	{
		do
		{
			char inPathFile[100];
			strcpy(inPathFile, inPath);
			strcat(inPathFile, fileinfo.name);
			inFileList->add(inPathFile);
		} while (!(_findnext(handle, &fileinfo)));
		_findclose(handle);
	}
}

void LoadDict(BalancedBinaryTree* dict, char* path)
{
	string lineData;
	ifstream readDict(path);
	int max_num = 275908;
	const int max_len = 50;
	char** temp = new char* [max_num];
	memset(temp, 0, sizeof(char*) * max_num);
	for (int i = 0; i < max_num; i++)
	{
		temp[i] = new char[max_len];
		memset(temp[i], 0, sizeof(char) * max_len);
	}
	max_num = 0;
	while (readDict.getline(temp[max_num], 50))
	{
		//cout << temp[max_num] << ",";
		max_num++;
	}
	cout << "dict num: " << max_num << endl;
	readDict.close();
	// 折半插入
	bool adjust = false;
	//for (int i = 0; i < max_num; i++)
	//	dict->InsertNode(temp[i], &adjust);
	int usedLength = 1;
	int* nextIndex = new int[usedLength];
	memset(nextIndex, 0, sizeof(int) * usedLength);
	nextIndex[0] = (0 + max_num) / 2;
	int success = 0;
	while (success < max_num)
	{
		int* nextIndexTemp = new int[usedLength * 2];
		memset(nextIndexTemp, 0, sizeof(int) * usedLength * 2);
		int k = 0;
		// 一边插入，一边更新下一个待插入序列
		for (int i = 0; i < usedLength; i++)
		{
			if (dict->InsertNode(temp[nextIndex[i]], &adjust))
				success++;
			//cout << temp[nextIndex[i]] << ", ";
			if (i == 0 && i == usedLength - 1)
			{
				nextIndexTemp[k++] = nextIndex[0] / 2;
				nextIndexTemp[k++] = (nextIndex[0] + max_num) / 2;
			}
			else if (i == usedLength - 1 && i != 0)
			{
				nextIndexTemp[k++] = (nextIndex[i] + max_num) / 2;
			}
			else
			{
				if (i == 0)
				{
					nextIndexTemp[k++] = nextIndex[0] / 2;
					if (usedLength == 1)
						break;
				}
				int mid = (nextIndex[i] + nextIndex[i + 1] + 1) / 2;
				nextIndexTemp[k++] = (nextIndex[i] + mid) / 2;
				nextIndexTemp[k++] = (mid + nextIndex[i + 1]) / 2;
			}
		}
		delete[] nextIndex;
		nextIndex = nextIndexTemp;
		usedLength *= 2;
	}
	for (int i = 0; i < max_num; i++)
		delete[] temp[i];
	delete[] temp;

}

int main()
{
	system("chcp 65001");

	// Load dictionary from the given file .dic
	char dictPath[] = ".\\dictionary\\mydict.dic";
	BalancedBinaryTree* dict = new BalancedBinaryTree();
	LoadDict(dict, dictPath);
	cout << "Load dictionary from the given file .dic" << endl;
	
	// Get all .txt file name
	char txtPathName[15] = u8".\\input\\";
	char txtFormat[10] = u8".txt";
	CharStringLink* txtFileList = new CharStringLink();
	GetFileList(txtFileList, txtPathName, txtFormat);
	cout << "Get all .txt file name" << endl;
	
	// Build document list
	BaseCharString* pTxtList = txtFileList->getBase()->next;
	while (pTxtList != NULL)
	{
		char txtNameTemp[40];
		pTxtList->data->toChar(txtNameTemp);
		int docID = CharToInt(txtNameTemp);
		char wordTemp[40];
		ifstream readTxt(txtNameTemp);
		//cout << txtNameTemp << endl;
		while (readTxt.getline(wordTemp, 40))
		{
			//cout << wordTemp << ",";
			dict->UpdateNode(wordTemp, docID);
		}
		readTxt.close();
		pTxtList = pTxtList->next;
	}
	cout << "Build document list" << endl;

	// Deal with query1.txt
	char query1File[] = "query1.txt";
	char result1File[] = "result1.txt";
	ifstream readQuery1(query1File);
	ofstream writeResult1(result1File);

	char sentTemp[100];
	while (readQuery1.getline(sentTemp, 100))
	{
		// Read a line
		int lenSent = strlen(sentTemp);
		doc* searchResult = NULL;
		for (int i = 0; i < lenSent; i++)
		{
			// Read a word
			char aWord[50] = { '\0' };
			int end = i + 1;
			while (end < lenSent && sentTemp[end] != ' ')
				end++; // The position of space
			for (int j = 0; i + j < end; j++)
			{
				aWord[j] = sentTemp[i + j];
			}
			i = end;
			//cout << aWord << endl;
			//writeResult1 << aWord << endl;

			// Get document list of aWord
			if (searchResult == NULL)
			{
				PNode temp = dict->SearchNode(aWord);
				if (temp)
					searchResult = temp->article;
			}
			else
			{
				doc* temp = searchResult;
				while (temp->next != NULL)
					temp = temp->next;
				temp->next = dict->SearchNode(aWord)->article;
			}
		}

		// Write result to outfile
		while (searchResult)
		{
			writeResult1 << "(" << searchResult->docID << "," << searchResult->times << ")";
			cout << "(" << searchResult->docID << "," << searchResult->times << ")";
			searchResult = searchResult->next;
			if (searchResult)
			{
				writeResult1 << " ";
				cout << " ";
			}
		}
		writeResult1 << "\n";
		cout << "\n";
	}
	readQuery1.close();
	writeResult1.close();


}