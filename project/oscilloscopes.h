#ifndef OSCILLOSCOPES_OSCILLOSCOPE_CPP_H_
#define OSCILLOSCOPES_OSCILLOSCOPE_CPP_H_

#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <thread>
#include <chrono>
#include <exception>
#include <string>

namespace oscilloscopes
{

    constexpr int OSCILLOSCOPE_ERROR_INVALID_PARAM = 1;
    constexpr int OSCILLOSCOPE_SUCCESS             = 0;
    constexpr int OSCILLOSCOPE_ERROR_ACCESS        = -3;
    constexpr int OSCILLOSCOPE_ERROR_NO_DEVICE     = -4;
    constexpr int OSCILLOSCOPE_ERROR_TIMEOUT       = -7;
    constexpr int OSCILLOSCOPE_ERROR_NO_MEM        = -11;
    constexpr int OSCILLOSCOPE_ERROR_OTHER         = -99;

    /** @brief Ошибка матрицы, с выводом сообщения
     * */
    class OscilloscopeException : public std::exception
    {
    public:
        explicit OscilloscopeException( const std::string& msg ) : m_msg(msg) {}
        const char *what() const noexcept override
        {
            return m_msg.c_str();
        }

    private:
        std::string m_msg;
    };

    class InvalidParamOscilloscope : public OscilloscopeException
    {
    public:
        InvalidParamOscilloscope( const std::string& methodName ) :
            OscilloscopeException( ( "В методе " + methodName + " проблема с параметрами/ом" ) ) {}
    };

    class AccessOscilloscopeException : public OscilloscopeException
    {
    public:
        AccessOscilloscopeException( const std::string& methodName ) :
            OscilloscopeException( ( "В методе " + methodName + " проблема с доступом" ) ) {}
    };

    class NoDeviceOscilloscope : public OscilloscopeException
    {
    public:
        NoDeviceOscilloscope( const std::string &methodName ) :
            OscilloscopeException( ( "В методе " + methodName + " не обнаружено устройств" ) ) {}
    };

    class TimeOut : public OscilloscopeException
    {
    public:
        TimeOut( const std::string &methodName ) :
            OscilloscopeException( ( "В методе " + methodName + " произошел timeout" ) ) {}
    };

    class MemoryException : public OscilloscopeException
    {
    public:
        MemoryException( const std::string& methodName ) :
            OscilloscopeException( ( "В методе " + methodName + " под параметр нет памяти")) {}
    };

    class OtherException : public OscilloscopeException
    {
    public:
        OtherException( const std::string& methodName ) :
            OscilloscopeException( ( "В методе " + methodName + " произошло что-то неизвестное")) {}
    };

    /** @brief THROW
     * */
    template <typename T = uint8_t>
    void THROW( const int& r, const std::string& fooName, T *data = nullptr )
    {
        if( r == 0 )
            return;

        if( data != nullptr )
            delete []data;

        switch(r)
        {
            case OSCILLOSCOPE_ERROR_INVALID_PARAM:
                throw InvalidParamOscilloscope( fooName );
            case OSCILLOSCOPE_ERROR_ACCESS:
                throw AccessOscilloscopeException(fooName);
            case OSCILLOSCOPE_ERROR_NO_DEVICE:
                throw NoDeviceOscilloscope(fooName);
            case OSCILLOSCOPE_ERROR_TIMEOUT:
                throw TimeOut(fooName);
            case OSCILLOSCOPE_ERROR_NO_MEM:
                throw MemoryException(fooName);
            case OSCILLOSCOPE_ERROR_OTHER:
                throw OtherException(fooName);
            default:
                throw OscilloscopeException( "В методе " + fooName + " произошло что-то неизвестное!!!" );
        };
    }

    struct OscSignal
    {
        size_t _sampleRate;
        size_t _inputLevel;
        size_t _signalSize;
        std::vector<float> _signal;

        OscSignal() : _sampleRate(0), _inputLevel(0), _signalSize(0), _signal(0) {}

        OscSignal( const OscSignal& rhs )
        {
            this->_sampleRate = rhs._sampleRate;
            this->_inputLevel = rhs._inputLevel;
            this->_signalSize = rhs._signalSize;
            for( size_t i = 0; i < this->_signalSize; ++i )
                this->_signal.push_back( rhs._signal[i] );
        }

        OscSignal& operator=( const OscSignal& rhs )
        {
            if( &rhs != this )
            {
                this->_sampleRate = rhs._sampleRate;
                this->_inputLevel = rhs._inputLevel;
                this->_signalSize = rhs._signalSize;
                for( size_t i = 0; i < this->_signalSize; ++i )
                    this->_signal.push_back( rhs._signal[i] );
            }
            return *this;
        }

        ~OscSignal()
        {
            if( !_signal.empty() )
                _signal.clear();
        }
    };

    using OscSigframe = std::map<uint8_t, OscSignal>;

    class Oscilloscope
    {
    public:
        virtual ~Oscilloscope() {};
        virtual const size_t getChannelsSize() const = 0;
        virtual const std::string whoAmI() const = 0;

        virtual size_t setSampleRate( const size_t& ) = 0;
        virtual const size_t getSampleRate() = 0;
        virtual const std::vector<size_t> getRangeSampleRate() const = 0;

        virtual size_t setInputLevel( const uint8_t&, const size_t& ) = 0;
        virtual const size_t getInputLevel( const uint8_t& ) = 0;
        virtual const std::vector<size_t> getRangeInputLevel() const = 0;

        virtual OscSigframe getSignalFrame( const size_t& ) = 0;
        virtual const std::vector<size_t> getRangeSignalFrame() const = 0;

        virtual OscSignal getSignalFromTrigger( const uint8_t&, const float&, const uint8_t& ) = 0;

    };

};  // oscilloscopes

#endif  // OSCILLOSCOPES_OSCILLOSCOPE_CPP_H_
