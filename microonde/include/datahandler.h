#ifndef datahandler_h
#define datahandler_h

#include <vector>
#include <iostream>
#include <string>
#include <fstream>

//Convertitori
template <class C>
struct CONV
{
    static C s_conv_imp(std::string elem_str) {};
};
template <>
struct CONV<double>
{
    static double s_conv_imp(std::string elem_str) { return std::stod(elem_str); }
};
template <>
struct CONV<float>
{
    static float s_conv_imp(std::string elem_str) { return std::stof(elem_str); }
};
template <>
struct CONV<int>
{
    static int s_conv_imp(std::string elem_str) { return std::stoi(elem_str); }
};
template <class C>
C s_conv(std::string elem_str) { return CONV<C>::s_conv_imp(elem_str); }


//Main dataframe module
template <class T>
class dataframe
{
private:
    std::vector<std::vector<T>*> df;
public:
    dataframe();
    ~dataframe();

    //Data insertion
    void read_txt(std::string path, char element_separator = ' ', char line_separator = '\n', bool as_keys = false);

    //Utils
    void print_line(int line);
    std::vector<T> copy_row(int line);
    void clear();
    
    //Stats
    T min();
    T min(int index, char mode='r'); //modes: r->riga, c->colonna
    T max();
    T max(int index, char mode='r'); //modes: r->riga, c->colonna
};

template <class T>
dataframe<T>::dataframe()
{
    std::vector<T>* row = new std::vector<T>;
    this->df.push_back(row);
}

template <class T>
dataframe<T>::~dataframe()
{
    for (size_t i = 0; i < this->df.size(); i++)
    {
        delete this->df.at(i);
    }
    
}

template <class T>
void dataframe<T>::read_txt(std::string path, char element_separator, char line_separator, bool as_keys)
{
    std::fstream datafile;
    datafile.open(path, std::ios::in);
    if (datafile.is_open())
    {
        double row_iter = 0;
        char in_char;
        std::string element_str;
        T element;
        while (datafile >> std::noskipws >> in_char)
        {
            if (as_keys)
            {
                //first line is keys, so we skip it
                //skip unitl end of line
                while (in_char != line_separator)
                {
                    datafile >> std::noskipws >> in_char;
                }
            }
            
            if (in_char != line_separator) //Leggi fino alla fine della riga (o fine del file)
            {
                if (in_char != element_separator)
                {
                    element_str += in_char;
                }//Alla fine di questo cilco abbiamo letto un elemento
                else
                {
                    element = s_conv<T>(element_str); //Convertiamo la stringa nel elemento del tipo del template (T)
                    this->df.at(row_iter)->push_back(element); //Aggiuge l'elemento alla riga attuale
                    element_str = "";
                }  
            }
            else
            {
                //Aggiunta dell'ultimo elemento della riga
                element = s_conv<T>(element_str);
                this->df.at(row_iter)->push_back(element);

                this->df.push_back(new std::vector<T>); //Aggiunge una nuova riga
                row_iter++;
                element_str = "";
            }
        }
        element = s_conv<T>(element_str);
        this->df.at(row_iter)->push_back(element);
    }
    datafile.close();
}


//Utils *******************************************
template <class T>
void dataframe<T>::print_line(int line)
{
    std::vector<T> line_vector(*this->df.at(line));
    for (size_t i = 0; i < line_vector.size(); i++)
    {
        if (i < line_vector.size()-1)
        {
            std::cout << line_vector.at(i) << ", ";
        }
        else
        {
            std::cout << line_vector.at(i) << "\n";
        }
    }
}

template <class T>
std::vector<T> dataframe<T>::copy_row(int line)
{
    return *(this->df.at(line));
}

template <class T>
void dataframe<T>::clear()
{
    //delete all rows (vector pointers)
    for (size_t i = 0; i < this->df.size(); i++)
    {
        delete this->df.at(i);
    }
}


//Stats ********************************************
template <class T>
T dataframe<T>::min()
{
    T min = (this->df.at(0))->at(0);
    for (size_t i = 0; i < this->df.size(); i++) //loop rows
    {
        for (size_t j = 0; j < (this->df.at(i))->size(); j++) //loop col
        {
            if ((this->df.at(i))->at(j) < min)
            {
                min = (this->df.at(i))->at(j);
            }
        }
    }
    return min;
}
template <class T>
T dataframe<T>::min(int index, char mode)
{
    if (mode == 'r') //minimo di una riga
    {
        T min = (this->df.at(index))->at(0);
        for (size_t i = 0; i < (this->df.at(index))->size(); i++)
        {
            if ((this->df.at(index))->at(i) < min)
            {
                min = (this->df.at(index))->at(i);
            }
            
        }
        return min;
    }
    if (mode == 'c') //minimo di una colonna
    {
        T min = (this->df.at(0))->at(index);
        for (size_t i = 0; i < this->df.size(); i++)
        {
            if ((this->df.at(i))->at(index) < min)
            {
                min = (this->df.at(i))->at(index);
            }
            
        }
        return min;
    }
    else {
        std::cout << "Errore: " << mode << " non è un parametro valido, i parametri validi sono: r=row e c=column";
        exit(1);
    }
}
template <class T>
T dataframe<T>::max()
{
    T max = (this->df.at(0))->at(0);
    for (size_t i = 0; i < this->df.size(); i++) //loop rows
    {
        for (size_t j = 0; j < (this->df.at(i))->size(); j++) //loop col
        {
            if ((this->df.at(i))->at(j) > max)
            {
                max = (this->df.at(i))->at(j);
            }
        }
    }
    return max;
}
template <class T>
T dataframe<T>::max(int index, char mode)
{
    if (mode == 'r') //massimo di una riga
    {
        T max = (this->df.at(index))->at(0);
        int a = (this->df.at(index))->size();
        for (size_t i = 0; i < (this->df.at(index))->size(); i++)
        {
            if ((this->df.at(index))->at(i) > max)
            {
                max = (this->df.at(index))->at(i);
            }
            
        }
        return max;
    }
    if (mode == 'c') //massimo di una colonna
    {
        T max = (this->df.at(0))->at(index);
        for (size_t i = 0; i < this->df.size(); i++)
        {
            if ((this->df.at(i))->at(index) > max)
            {
                max = (this->df.at(i))->at(index);
            }
            
        }
        return max;
    }
    else {
        std::cout << "Errore: " << mode << " non è un parametro valido, i parametri validi sono: r=row e c=column";
        exit(1);
    }
}



#endif