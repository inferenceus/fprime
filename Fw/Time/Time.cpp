#include <Fw/Time/Time.hpp>
#include <Fw/Types/BasicTypes.hpp>

namespace Fw {

    Time::Time(void) : m_seconds(0), m_useconds(0), m_timeBase(TB_NONE), m_timeContext(0)  {
    }

    Time::~Time(void) {
    }

    Time::Time(const Time& other) : Serializable() {
        this->set(other.m_timeBase,other.m_timeContext,other.m_seconds,other.m_useconds);
    }

    Time::Time(U32 seconds, U32 useconds) {
        this->set(TB_NONE,0,seconds,useconds);
    }

    Time::Time(TimeBase timeBase, U32 seconds, U32 useconds) {
        this->set(timeBase,0,seconds,useconds);
    }

    void Time::set(U32 seconds, U32 useconds) {
        this->set(TB_NONE,0,seconds,useconds);
    }

    void Time::set(TimeBase timeBase, U32 seconds, U32 useconds) {
        this->set(timeBase,0,seconds,useconds);
    }

    Time::Time(TimeBase timeBase, FwTimeContextStoreType context, U32 seconds, U32 useconds) {
        this->set(timeBase,context,seconds,useconds);
    }

    void Time::set(TimeBase timeBase, FwTimeContextStoreType context, U32 seconds, U32 useconds) {
        this->m_timeBase = timeBase;
        this->m_timeContext = context;
        this->m_useconds = useconds;
        this->m_seconds = seconds;
    }

    bool Time::operator==(const Time& other) const {
        return (Time::compare(*this,other) == EQ);
    }

    bool Time::operator!=(const Time& other) const {
        return (Time::compare(*this,other) != EQ);
    }

    bool Time::operator>(const Time& other) const {
        return (Time::compare(*this,other) == GT);
    }

    bool Time::operator<(const Time& other) const {
        return (Time::compare(*this,other) == LT);
    }

    bool Time::operator>=(const Time& other) const {
        Time::Comparison c = Time::compare(*this,other);
        return ((GT == c) or (EQ == c));
    }

    bool Time::operator<=(const Time& other) const {
        Time::Comparison c = Time::compare(*this,other);
        return ((LT == c) or (EQ == c));
    }

    SerializeStatus Time::serialize(SerializeBufferBase& buffer) const {
        // serialize members
        SerializeStatus stat = Fw::FW_SERIALIZE_OK;
#if FW_USE_TIME_BASE
        stat = buffer.serialize(static_cast<FwTimeBaseStoreType>(this->m_timeBase));
        if (stat != FW_SERIALIZE_OK) {
            return stat;
        }
#endif

#if FW_USE_TIME_CONTEXT
        stat = buffer.serialize(this->m_timeContext);
        if (stat != FW_SERIALIZE_OK) {
            return stat;
        }
#endif

        stat = buffer.serialize(this->m_seconds);
        if (stat != FW_SERIALIZE_OK) {
            return stat;
        }

        return buffer.serialize(this->m_useconds);
    }

    SerializeStatus Time::deserialize(SerializeBufferBase& buffer) {

        SerializeStatus stat = Fw::FW_SERIALIZE_OK;
#if FW_USE_TIME_BASE
        FwTimeBaseStoreType deSer;

        stat = buffer.deserialize(deSer);
        if (stat != FW_SERIALIZE_OK) {
            return stat;
        }

        this->m_timeBase = static_cast<TimeBase>(deSer);
#else
        this->m_timeBase = TB_NONE;
#endif
#if FW_USE_TIME_CONTEXT
        stat = buffer.deserialize(this->m_timeContext);
        if (stat != FW_SERIALIZE_OK) {
            return stat;
        }
#else
        this->m_timeContext = 0;
#endif
        stat = buffer.deserialize(this->m_seconds);
        if (stat != FW_SERIALIZE_OK) {
            return stat;
        }

        return buffer.deserialize(this->m_useconds);
    }

    U32 Time::getSeconds(void) const {
        return this->m_seconds;
    }

    U32 Time::getUSeconds(void) const {
        return this->m_useconds;
    }

    TimeBase Time::getTimeBase(void) const {
        return this->m_timeBase;
    }

    U8 Time::getContext(void) const {
        return this->m_timeContext;
    }

    Time Time ::
      zero(TimeBase timeBase)
    {
      Time time(timeBase,0, 0, 0);
      return time;
    }

    Time::Comparison Time ::
      compare(
          const Time &time1,
          const Time &time2
      )
    {
#if FW_USE_TIME_BASE
      FW_ASSERT(time1.getTimeBase() == time2.getTimeBase(), time1.getTimeBase(), time2.getTimeBase() );
#endif
#if FW_USE_TIME_CONTEXT
      FW_ASSERT(time1.getContext() == time2.getContext(), time1.getContext(), time2.getContext() );
#endif
      const U32 s1 = time1.getSeconds();
      const U32 s2 = time2.getSeconds();
      const U32 us1 = time1.getUSeconds();
      const U32 us2 = time2.getUSeconds();

      if (s1 < s2) {
        return LT;
      } else if (s1 > s2) {
        return GT;
      } else if (us1 < us2) {
        return LT;
      } else if (us1 > us2) {
        return GT;
      } else {
        return EQ;
      }
    }

    Time Time ::
      add(
        Time& a,
        Time& b
      ) 
    {
#if FW_USE_TIME_BASE
      FW_ASSERT(a.getTimeBase() == b.getTimeBase(), a.getTimeBase(), b.getTimeBase() );
#endif
#if FW_USE_TIME_CONTEXT
      FW_ASSERT(a.getContext() == b.getContext(), a.getContext(), b.getContext() );
#endif
      U32 seconds = a.getSeconds() + b.getSeconds();
      U32 uSeconds = a.getUSeconds() + b.getUSeconds();
      FW_ASSERT(uSeconds < 1999999);
      if (uSeconds >= 1000000) {
        ++seconds;
        uSeconds -= 1000000;
      }
      Time c(a.getTimeBase(),a.getContext(),seconds,uSeconds);
      return c;
    }

    Time Time ::
      sub(
        Time& minuend, //!< Time minuend
        Time& subtrahend //!< Time subtrahend
    )
    {
#if FW_USE_TIME_BASE
      FW_ASSERT(minuend.getTimeBase() == subtrahend.getTimeBase(), minuend.getTimeBase(), subtrahend.getTimeBase());
#endif
#if FW_USE_TIME_CONTEXT
      FW_ASSERT(minuend.getContext() == subtrahend.getContext(), minuend.getContext(), subtrahend.getContext());
#endif
      // Assert minuend is greater than subtrahend
      FW_ASSERT(minuend > subtrahend);

      U32 seconds = minuend.getSeconds() - subtrahend.getSeconds();
      U32 uSeconds;
      if (subtrahend.getUSeconds() > minuend.getUSeconds()) {
          seconds--;
          uSeconds = minuend.getUSeconds() + 1000000 - subtrahend.getUSeconds();
      } else {
          uSeconds = minuend.getUSeconds() - subtrahend.getUSeconds();
      }
      return Time(minuend.getTimeBase(), minuend.getContext(), seconds, static_cast<U32>(uSeconds));
    }

    void Time::add(U32 seconds, U32 useconds) {
        this->m_seconds += seconds;
        this->m_useconds += useconds;
        FW_ASSERT(this->m_useconds < 1999999,this->m_useconds);
        if (this->m_useconds >= 1000000) {
          ++this->m_seconds;
          this->m_useconds -= 1000000;
        }
    }

    void Time::setTimeBase(TimeBase timeBase) {
        this->m_timeBase = timeBase;
    }

    void Time::setTimeContext(FwTimeContextStoreType context) {
        this->m_timeContext = context;
    }

#ifdef BUILD_UT
    std::ostream& operator<<(std::ostream& os, const Time& val) {

        os << "(" << val.getTimeBase() << "," << val.getUSeconds() << "," << val.getSeconds() << ")";
        return os;
    }
#endif

}
