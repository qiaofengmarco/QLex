#ifndef MY_PREPROCESS_HPP
#define MY_PREPROCESS_HPP

/***********************************************************
From: Jiahuan Song,
      School of Computer Science and Engineering,
      Southeast University, Jiulonghu Campus,
      Nanjing, China
************************************************************/

#include "Headers.h"

class Preprocessor
{
public:

    Preprocessor()
    {
        cout << "Start preprocessing..." << endl;
        change("./lex.l");
        cout << "Finish preprocessing..." << endl;
    };
    
    Preprocessor(string FileName)
    {
        cout << "Start preprocessing..." << endl;
        change(FileName);
        cout << "Finish preprocessing..." << endl;
    }

    void getResult(vector<string> &result)
    {
        result.resize(re.size());
        for (unsigned int i = 0; i < re.size(); i++)
            result[i] = re[i];
    }

private:

    static vector<string> re;//用于保存规范化正规表达式的结果
    //static vector<string> act;
    static map<string, string> mapRe;//用于保存声明部分的正规定义（RE是已经进行过规范化的）
    static fstream f1, f2;

    void change(string FileName)
    {
        f1.open(FileName, ios::in);//打开.l文件
        f2.open("./lex.yy.c", ios::out);//建立临时.c文件
        string line;//缓存每一行的内容

        getline(f1, line);
        skipAndCopy(line);//拷贝%{ %}之间的部分
        reProcess(line);//声明部分正规定义的处理
        skipAndCopy(line);//拷贝%{ %}之间的部分
        modeProcess(line);//转换规则部分的处理
        copy(line);//辅助函数部分的直接拷贝
        f1.close();
        f2.close();
    }

    void skipAndCopy(string &line)//拷贝%{ %}之间的部分
    {
	    //跳过空行
	    while (judgeNull(line))
		    getline(f1, line);

		//拷贝%{ %}之间的部分
	    if (line == "%{")
	    {
		    getline(f1, line);
		    while (line != "%}")
		    {
			    f2 << line << "\n";
			    getline(f1, line);
		    }

		    getline(f1, line);
		    while (judgeNull(line))
			    getline(f1, line);
	    }
    }

    void reProcess(string &line)//声明部分正规定义的处理
    {
	    string reName, reContent;
	    int count, nameStart, nameEnd, contentStart, contentEnd;

	    while (true)
	    {
		    if (line == "%{")
			    break;
		    else if (line == "%%")
			    break;
		    else if (judgeNull(line))
		    {
			    getline(f1, line);
			    continue;
		    }
		    else
		    {
			    count = 0;
			    while (true)
			    {
				    if (line[count] != ' ' && line[count] != '\t')
				    {
					    nameStart = count;
					    count++;
					    break;
				    }

				    count++;
			    }

			    while (true)
			    {
				    if (line[count] == ' ' || line[count] == '\t')
				    {
					    nameEnd = count-1;
					    count++;
					    break;
				    }

				    count++;
			    }

			    while (true)
			    {
				    if (line[count] != ' ' && line[count] != '\t')
				    {
					    contentStart = count;
					    count++;
					    break;
				    }

				    count++;
			    }

			    while (line[count] != '\0')
				    count++;

			    count--;
			    while (line[count] == ' ' || line[count] == '\t')
				    count--;
			    contentEnd = count;

			    reName = line.substr(nameStart, nameEnd - nameStart + 1);
			    reContent = line.substr(contentStart, contentEnd - contentStart + 1);
			    mapRe.insert(pair<string, string>(reName, process(reContent)));
		    }

		    getline(f1, line);
	    }
	

    }

    void modeProcess(string &line)//转换规则部分的处理
    {
	    getline(f1, line);
	
	    string regu;
	    string action;
	    int reStart;
	    int reEnd;
	    int actionStart;
	    int actionEnd;
	    int count;
	    int flag;
		int pid = 0;
		
		f2 << "int act(int pid) {" << endl;
		
		
	    while (line != "%%")
	    {
		    if (judgeNull(line))
		    {
			    getline(f1, line);
			    continue;
		    }
		    else
		    {
			    count = 0;

			    while (true)
			    {
				    if (line[count] != ' ' && line[count] != '\t')
				    {
					    reStart = count;
					    count++;
					    break;
				    }

				    count++;
			    }

			    while (line[count] != '\0')
				    count++;

			    count--;
			    while(line[count] != '}')
				    count--;
			    actionEnd = count;
			
			    flag = 1;
			    while (flag != 0)
			    {
				    count--;
				    if (line[count] == '\'')
					    count -= 4;
				    else if (line[count] == '}')
					    flag++;
				    else if (line[count] == '{')
					    flag--;
			    }
			    actionStart = count;

			    count--;
			    while (line[count] == ' ' || line[count] == '\t')
				    count--;
			    reEnd = count;

			    regu = line.substr(reStart, reEnd - reStart + 1);
				re.push_back(process(regu));
			    action = line.substr(actionStart, actionEnd - actionStart + 1);
			    //act.push_back(action);
				if(pid > 0)
					f2 << "else ";
					
				f2 << "if (pid == " << pid++ << ") " << action << endl;
		    }

		    getline(f1, line);
	    }
        f2 << "else if (pid == -1) { return -1; }" << endl;
        f2 << "return 0;\n}" << endl;
    }

    void copy(string &line)//辅助函数部分的直接拷贝
    {
	    while (!f1.eof())
	    {
		    getline(f1, line);
		    f2 << line << "\n";
	    }
    }

    bool judgeNull(const string &str)//判断字符串是否为空或只有空格和tab
    {
	    for (int i = 0; str[i] != '\0'; i++)
		    if (str[i] != ' ' && str[i] != '\t')
			    return false;

	    return true;
    }

    string process(const string &str)//非规范RE到规范RE的转化
    {
	    string resultStr = "";
	    string tempStr="";

	    unsigned int count = 0;
	    while (str[count] != '\0')
	    {
		    if (str[count] == '"')
		    {
			    tempStr = "(";

			    count++;
			    while (str[count] != '"')
			    {
					if(str[count] == '('||str[count] == ')'||str[count] == '*'||str[count] == '|'||str[count] == '\\')
						tempStr += '\\';
				    tempStr += str[count];
				    count++;
			    }
			
			    tempStr += ")";
			    resultStr += tempStr;
		    }
		    else if (str[count] == '[')
		    {
			    tempStr = "(";

			    count++;
			    if (str[count] == '^')
			    {
				    string chs="";

				    count++;
				    while (str[count] != ']')
				    {
					    if (str[count] == '\\')
						    count++;
					
					    chs += str[count];
	
					    count++;
				    }

				    int ch = 0;
				    int result;
				    while (ch <= 127)
				    {
					    result = chs.find((char)ch); //查找ch
					    if (result == -1 && ch != '$')
						{
							if(ch == '('||ch == ')'||ch == '*'||ch == '|'||ch == '\\')
								tempStr += '\\';
							tempStr = tempStr + (char)ch + '|';
						}
						
						/*if(ch == 255) break;*/
					    ch++;
				    }

				    tempStr = tempStr.substr(0, tempStr.length() - 1);
			    }
			    else
			    {
				    while (str[count] != ']')
				    {
					    if (str[count + 1] == '-' && ((isDigital(str[count]) && isDigital(str[count+2])) || (isLetter(str[count]) && isLetter(str[count + 2])) ) )
					    {
						    char ch = str[count];
						    while (ch <= str[count + 2])
						    {
							    tempStr = tempStr + ch + '|';
							    ch++;
						    }
						    count += 2;
						
					    }
					    else if(str[count] == '\\')
					    {
						    count++;

						    if(str[count]=='t')
							    tempStr = tempStr + '\t' + '|';
						    else if(str[count] == 'v')
							    tempStr = tempStr + '\v' + '|';
						    else if(str[count] == 'n')
							    tempStr = tempStr + '\n' + '|';
						    else if(str[count] == 'f')
							    tempStr = tempStr + '\f' + '|';
					    }
					    else
					    {
						    tempStr = tempStr + str[count] + '|';
					    }

					    count++;
				    }
				    tempStr = tempStr.substr(0, tempStr.length() - 1);
			    }
			
			    tempStr += ")";
			    resultStr += tempStr;
		    }
		    else if (str[count] == '{')
		    {
			    string subStr = "";

			    bool flag = false;
			    int pos = 0;
			    count++;
			    while (str[count] != '}')
			    {
				    if (str[count] == ',')
				    {
					    flag = true;
					    pos = count;
				    }
					
				    subStr += str[count];
				    count++;
			    }

			    if (isLetter(subStr[0]))
			    {
				    tempStr = "(" + mapRe.find(subStr)->second + ")";
				    resultStr += tempStr;
			    }
			    else if (isDigital(subStr[0]))
			    {
				    if (flag)
				    {
					    int lengthOfEnd = count - pos - 1;
					    int lengthOfStart = subStr.length() - lengthOfEnd - 1;
					
					    int numStart = stoi(subStr.substr(0, lengthOfStart));
					    int numEnd = stoi(subStr.substr(lengthOfStart + 1, lengthOfEnd));

					    resultStr = resultStr.substr(0, resultStr.length() - tempStr.length());
					
					    string temp = "";
					    for (int i = 0; i < numStart; i++)
					    {
						    temp = temp + tempStr;
					    }

					    string tempString = "(" + temp + ")";
					    for (int j = numStart+1; j <= numEnd; j++)
					    {
						    temp = temp + tempStr;
						    tempString = tempString + "|" + "(" + temp + ")";
					    }

					    tempStr = "(" + tempString + ")";
					    resultStr += tempStr;
				    }
				    else
				    {
					    int num = stoi(subStr);

					    resultStr = resultStr.substr(0, resultStr.length() - tempStr.length());

					    string temp = "";
					    for (int i = 0; i < num; i++)
					    {
						    temp = temp + tempStr;
					    }
					    tempStr = "(" + temp + ")";
					
					    resultStr += tempStr;
				    }
			    }


		    }
		    else if (str[count] == '(')
		    {
			    string subStr = "";
			    count++;
			    int flag = 1;
			    while (flag != 0)
			    {
				    if (str[count] == '[')
				    {
					    subStr += str[count];
					    count++;
					    while (str[count] != ']')
					    {
						    subStr += str[count];
						    count++;
					    }
					    subStr += str[count];
					    count++;
				    }
				    else if (str[count] == '\\')
				    {
					    subStr += str[count];
					    count++;
					    subStr += str[count];
					    count++;
				    }
				    else if (str[count] == '"')
				    {
					    subStr += str[count];
					    count++;
					    while (str[count] != '"')
					    {
						    subStr += str[count];
						    count++;
					    }
					    subStr += str[count];
					    count++;
				    }
				    else if(str[count] == '(')
				    {
					    subStr += str[count];
					    flag++;
					    count++;
				    }
				    else if (str[count] == ')')
				    {
					    flag--;
					    if (flag != 0)
					    {
						    subStr += str[count];
						    count++;
					    }
				    }
				    else
				    {
					    subStr += str[count];
					    count++;
				    }	
			    }

			    tempStr = "(" + process(subStr) + ")";
			    resultStr += tempStr;
		    }
		    else if (str[count] == '\\')
		    {
			    count++;
				if(str[count] == '('||str[count] == ')'||str[count] == '*'||str[count] == '|'||str[count] == '\\')
				{
					tempStr = '\\';
					tempStr += str[count];
				}
				else
				{
			    tempStr = str[count];
				}
				
			    resultStr += tempStr;
			
		    }
		    else if (str[count] == '+')
		    {
			    resultStr = resultStr.substr(0, resultStr.length() - tempStr.length());
			    tempStr = "(" + tempStr + tempStr + "*" + ")";
			    resultStr += tempStr;
		    }
		    else if (str[count] == '.')
		    {
			    tempStr = "(";

                int ch = 0;
			    while (ch <= 127)
			    {
				    if (ch != '$')
					{
						if(ch == '('||ch == ')'||ch == '*'||ch == '|'||ch == '\\')
								tempStr += '\\';
						tempStr = tempStr + (char)ch + '|';
					}
					
                    /*if(ch == 255) break;*/
                    ch++;
                    
			    }

			    tempStr = tempStr.substr(0, tempStr.length() - 1);

			    tempStr += ")";
			    resultStr += tempStr;
		    }
		    else if (str[count] == '?')
		    {
			    resultStr = resultStr.substr(0, resultStr.length() - tempStr.length());
			    tempStr = "(" + tempStr + "|" + "()" + ")";
			    resultStr += tempStr;
		    }
		    else
		    {
			    tempStr = str[count];
			    resultStr += tempStr;
		    }

		    count++;
	    }


	    return resultStr;
    }

    bool isDigital(const char ch)//判断是否为数字
    {
	    if (ch >= '0'&&ch <= '9')
		    return true;

	    return false;
    }

    bool isLetter(const char ch)//判断是否为字母
    {
	    if ((ch >= 'a'&&ch <= 'z') || (ch >= 'A'&&ch <= 'Z'))
		    return true;

	    return false;
    }
};

vector<string> Preprocessor::re {};
map<string, string> Preprocessor::mapRe {};
fstream Preprocessor::f1;
fstream Preprocessor::f2;

#endif