#ifndef PML_NAMEDTYPE_H
#define PML_NAMEDTYPE_H

#include <string>
#include <ostream>

template <typename T, typename Parameter>
class NamedType
{
    public:
        explicit NamedType(){}
        explicit NamedType(T const& value) : m_value(value){}
        explicit NamedType(T&& value) : m_value(std::move(value)){}
        T& Get() {return m_value;}
        T const& Get() const {return m_value;}

        bool operator<(T const& other)
        {
            return m_value < other.Get();
        }
        bool operator>(T const& other)
        {
            return other.Get() < m_value;
        }
        bool operator<=(T const& other)
        {
            return m_value <= other.Get();
        }
        bool operator>=(T const& other)
        {
            return !(*this < other);
        }
        bool operator==(T const& other)
        {
            return m_value == other.Get();
        }

        friend std::ostream& operator<<(std::ostream& os, const NamedType<T, Parameter>& type)
        {
            os << type.Get();
            return os;
        }
        friend bool operator==(const NamedType<T, Parameter>& A, const NamedType<T, Parameter>& B)
        {
            return A.Get() == B.Get();
        }
        friend bool operator<(const NamedType<T, Parameter>& A, const NamedType<T, Parameter>& B)
        {
            return A.Get() < B.Get();
        }
        friend bool operator>(const NamedType<T, Parameter>& A, const NamedType<T, Parameter>& B)
        {
            return A.Get() > B.Get();
        }

    private:
        T m_value;
};

#endif // PML_NAMEDTYPE
