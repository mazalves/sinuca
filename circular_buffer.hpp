/*
 * Copyright (C) 2010~2014  Marco Antonio Zanata Alves
 *                          (mazalves at inf.ufrgs.br)
 *                          GPPD - Parallel and Distributed Processing Group
 *                          Universidade Federal do Rio Grande do Sul
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include "./sinuca.hpp"

#ifndef _CIRCULAR_BUFFER_HPP_
#define _CIRCULAR_BUFFER_HPP_

// ============================================================================
///            -----------------
///   FRONT <- |0|1|2|3|4|5|6|7| <- BACK
/// (older)    -----------------    (newer)
///          BEG               END
template <class CB_TYPE>
class circular_buffer_t {
    public:
        uint32_t beg_index, end_index, size, capacity;
        CB_TYPE *data;

        // ====================================================================
        /// Methods
        // ====================================================================
        circular_buffer_t();
        ~circular_buffer_t();

        CB_TYPE& operator[](uint32_t index);

        /// Copy Assignment Operator
        circular_buffer_t& operator=(const circular_buffer_t& other)
        {
            ERROR_PRINTF("Avoid copying this structure for performance reasons.\n")
            return *this;
        }

        void allocate(uint32_t elements);

        uint32_t get_size();
        uint32_t get_capacity();
        bool is_full();
        bool is_empty();

        int32_t push_back(const CB_TYPE& new_element);
        CB_TYPE* front();
        CB_TYPE* back();
        void pop_front();
        void pop_push();
};


// ============================================================================
template <class CB_TYPE>
circular_buffer_t<CB_TYPE>::circular_buffer_t() {
    this->beg_index = 0;
    this->end_index = 0;
    this->size = 0;
    this->capacity = 0;

    this->data = NULL;
};

// ============================================================================
template <class CB_TYPE>
circular_buffer_t<CB_TYPE>::~circular_buffer_t() {
    if (this->data != NULL) {
        delete []data;
    }
};

// ============================================================================
template <class CB_TYPE>
void circular_buffer_t<CB_TYPE>::allocate(uint32_t elements) {
    this->capacity = elements;
    this->data = new CB_TYPE[this->capacity];

    ERROR_ASSERT_PRINTF(this->data != NULL, "Could not allocate the circular buffer size.\n")
};

// ============================================================================
template <class CB_TYPE>
CB_TYPE& circular_buffer_t<CB_TYPE>::operator[](uint32_t index) {
    ERROR_ASSERT_PRINTF(index < this->capacity, "Trying to access beyond the circular buffer size.\n")
    ERROR_ASSERT_PRINTF(this->data != NULL, "Trying to access beyond the circular buffer size.\n")

    uint32_t position = this->beg_index + index;
    if (position >= this->capacity)
        position -= this->capacity;
    // ~ uint32_t position = (this->beg_index + index) % this->capacity;

    return this->data[position];
};

// ============================================================================
template <class CB_TYPE>
uint32_t circular_buffer_t<CB_TYPE>::get_size() {
    return this->size;
};

// ============================================================================
template <class CB_TYPE>
uint32_t circular_buffer_t<CB_TYPE>::get_capacity() {
    return this->capacity;
};

// ============================================================================
template <class CB_TYPE>
bool circular_buffer_t<CB_TYPE>::is_full() {
    return (this->size == this->capacity);
};

// ============================================================================
template <class CB_TYPE>
bool circular_buffer_t<CB_TYPE>::is_empty() {
    return (this->size == 0);
};

// ============================================================================
/// Insert into the newest position
template <class CB_TYPE>
int32_t circular_buffer_t<CB_TYPE>::push_back(const CB_TYPE& new_element) {
    ERROR_ASSERT_PRINTF(this->data != NULL, "Trying to access beyond the circular buffer size.\n")

    int32_t virtual_position = POSITION_FAIL;

    if (!is_full()) {
        virtual_position = this->size;
        this->size++;
        this->data[end_index] = new_element;

        this->end_index++;
        if (this->end_index >= this->capacity)
            this->end_index = 0;
        // ~ this->end_index = (this->end_index + 1) % this->capacity;
    }

    return virtual_position;
};

// ============================================================================
/// Obtain the oldest package inside the circular buffer
/// Same as cb[0], but this is faster
template <class CB_TYPE>
CB_TYPE* circular_buffer_t<CB_TYPE>::front() {
    if (this->size == 0) {
        return NULL;
    }
    else {
        return &this->data[beg_index];
    }
};

// ============================================================================
template <class CB_TYPE>
/// Obtain the newest package inside the circular buffer
/// Same as cb[size], but this is faster
CB_TYPE* circular_buffer_t<CB_TYPE>::back() {
    if (this->size == 0) {
        return NULL;
    }
    else {
        uint32_t position = this->beg_index + this->size - 1;
        if (position >= this->capacity)
            position -= this->capacity;

        // ~ uint32_t position = (this->beg_index + this->size - 1) % this->capacity;
        return &this->data[position];
    }
};

// ============================================================================
/// Remove the oldest element
template <class CB_TYPE>
void circular_buffer_t<CB_TYPE>::pop_front() {
    if (this->size > 0) {
        this->size--;
        this->data[beg_index].package_clean();

        this->beg_index++;
        if (this->beg_index >= this->capacity)
            this->beg_index = 0;
        // ~ this->beg_index = (this->beg_index + 1) % this->capacity;
    }
};

// ============================================================================
/// Remove the oldest element and insert it into the newest position
template <class CB_TYPE>
void circular_buffer_t<CB_TYPE>::pop_push() {
    CB_TYPE older = this->data[beg_index];
    this->data[beg_index].package_clean();

    this->beg_index++;
    if (this->beg_index >= this->capacity)
        this->beg_index = 0;
    // ~ this->beg_index = (this->beg_index + 1) % this->capacity;

    this->data[end_index] = older;

    this->end_index++;
    if (this->end_index >= this->capacity)
        this->end_index = 0;
    // ~ this->end_index = (this->end_index + 1) % this->capacity;
};

#endif  // _CIRCULAR_BUFFER_HPP_
