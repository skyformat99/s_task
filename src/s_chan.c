#include "s_task.h"
#include <string.h>

/*******************************************************************/
/* chan                                                            */
/*******************************************************************/

/* Put element into chan */
void s_chan_put(__async__, s_chan_t *chan, const void *in_object) {
#if 1
    s_chan_put_n(__await__, chan, in_object, 1);
#else
    uint16_t end;
    while (chan->available_count >= chan->max_count) {
        s_event_wait(__await__, &chan->event);
    }

    end = chan->begin + chan->available_count;
    while (end > chan->max_count)
        end -= chan->max_count;

    memcpy((char*)&chan[1] + end * (size_t)chan->element_size, in_object, chan->element_size);
    ++chan->available_count;

    s_event_set(&chan->event);
#endif
}

/* Get element from chan */
void s_chan_get(__async__, s_chan_t *chan, void *out_object) {
#if 1
    s_chan_get_n(__await__, chan, out_object, 1);
#else
    while (chan->available_count <= 0) {
        s_event_wait(__await__, &chan->event);
    }

    memcpy(out_object, (char*)&chan[1] + chan->begin * (size_t)chan->element_size, chan->element_size);

    ++chan->begin;
    while (chan->begin > chan->max_count)
        chan->begin -= chan->max_count;
    --chan->available_count;

    s_event_set(&chan->event);
#endif
}


/* Put count of elements into chan */
void s_chan_put_n(__async__, s_chan_t *chan, const void *in_object, uint16_t number) {
    while(number > 0) {
        uint16_t begin;
        uint16_t end;
        uint16_t count;
        while (chan->available_count >= chan->max_count) {
            s_event_wait(__await__, &chan->event);
        }
        
        count = chan->max_count - chan->available_count;
        if(count > number)
            count = number;

        begin = chan->begin + chan->available_count;
        while (begin > chan->max_count)
            begin -= chan->max_count;
        
        end = begin + count;
        if(end >= chan->max_count) {
            uint16_t count0 = chan->max_count - begin;
            memcpy((char *)&chan[1] + begin * (size_t)chan->element_size, in_object, count0 * (size_t)chan->element_size);
            in_object = (const void *)((const char *)in_object + count0 * (size_t)chan->element_size);
            
            count0 = count - count0;
            memcpy((char *)&chan[1], in_object, count0 * (size_t)chan->element_size);
            in_object = (const void *)((const char *)in_object + count0 * (size_t)chan->element_size);
        }
        else {
            memcpy((char *)&chan[1] + begin * (size_t)chan->element_size, in_object, count * (size_t)chan->element_size);
            in_object = (const void *)((const char *)in_object + count * (size_t)chan->element_size);
        }
        
        chan->available_count += count;
        number -= count;

        s_event_set(&chan->event);
    }
}


/* Get count of elements from chan */
void s_chan_get_n(__async__, s_chan_t *chan, void *out_object, uint16_t number) {
    while(number > 0) {
        uint16_t end;
        uint16_t count;
        while (chan->available_count <= 0) {
            s_event_wait(__await__, &chan->event);
        }
        
        count = chan->available_count;
        if(count > number)
            count = number;
        
        end = chan->begin + count;
        if(end >= chan->max_count) {
            uint16_t count0 = chan->max_count - chan->begin;
            memcpy(out_object, (char *)&chan[1] + chan->begin * (size_t)chan->element_size, count0 * (size_t)chan->element_size);
            out_object = (void *)((char *)out_object + count0 * (size_t)chan->element_size);

            count0 = count - count0;
            memcpy(out_object, (char *)&chan[1], count0 * (size_t)chan->element_size);
            out_object = (void*)((char*)out_object + count0 * (size_t)chan->element_size);

            chan->begin = count0;
        }
        else {
            memcpy(out_object, (char *)&chan[1] + chan->begin * (size_t)chan->element_size, count * (size_t)chan->element_size);
            out_object = (void*)((char*)out_object + count * (size_t)chan->element_size);

            chan->begin = end;
        }

        chan->available_count -= count;
        number -= count;

        s_event_set(&chan->event);
    }
}