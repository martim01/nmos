#pragma once

class ResourceHolder
{
    public:
        ResourceHolder() : m_nVersion(0){}

        void ResourceUpdated()
        {
            if(m_nVersion < 255)
            {
                m_nVersion++;
            }
            else
            {
                m_nVersion = 0;
            }
        }
        unsigned char GetVersion() const
        {
            return m_nVersion;
        }
    protected:
        unsigned char m_nVersion;

};
