//==============================================================================
//
// Copyright (C) 2010, 2011
// Marco Antonio Zanata Alves
//
// GPPD - Parallel and Distributed Processing Group
// Universidade Federal do Rio Grande do Sul
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//
//==============================================================================
#include "../sinuca.hpp"
#include <string>

#ifdef DIRECTORY_CTRL_DEBUG
    #define DIRECTORY_CTRL_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define DIRECTORY_CTRL_DEBUG_PRINTF(...)
#endif

//==============================================================================
// directory_controller_line_t
//==============================================================================
directory_controller_line_t::directory_controller_line_t() {
    this->id_owner = 0;
    this->opcode_number = 0;
    this->opcode_address = 0;
    this->uop_number = 0;

    this->lock_type = LOCK_FREE;
    this->cache_request_order = utils_t::template_allocate_initialize_array<uint32_t>(sinuca_engine.cache_memory_array_size, 0);
    this->cache_requested = 0;

    this->initial_memory_operation = MEMORY_OPERATION_INST;
    this->initial_memory_address = 0;
    this->initial_memory_size = 0;

    this->born_cycle = sinuca_engine.get_global_cycle();
};

//==============================================================================
directory_controller_line_t::~directory_controller_line_t() {
    /// De-Allocate memory to prevent memory leak
    utils_t::template_delete_array<uint32_t>(cache_request_order);
};

//==============================================================================
void directory_controller_line_t::packager(uint32_t id_owner, uint64_t opcode_number, uint64_t opcode_address, uint64_t uop_number,
                                            lock_t lock_type,
                                            memory_operation_t initial_memory_operation, uint64_t initial_memory_address, uint32_t initial_memory_size) {
    this->id_owner = id_owner;
    this->opcode_number = opcode_number;
    this->opcode_address = opcode_address;
    this->uop_number = uop_number;

    this->lock_type = lock_type;

    // ~ this->cache_request_order = ;
    // ~ this->cache_requested = ;

    this->initial_memory_operation = initial_memory_operation;
    this->initial_memory_address = initial_memory_address;
    this->initial_memory_size = initial_memory_size;

    this->born_cycle = sinuca_engine.get_global_cycle();
};

//==============================================================================
std::string directory_controller_line_t::directory_controller_line_to_string() {
    std::string PackageString;
    PackageString = "";

    PackageString = PackageString + " DIR: Owner:" + utils_t::uint32_to_char(this->id_owner);
    PackageString = PackageString + " OPCode#:" + utils_t::uint64_to_char(this->opcode_number);
    PackageString = PackageString + " 0x" + utils_t::uint64_to_char(this->opcode_address);
    PackageString = PackageString + " UOP#:" + utils_t::uint64_to_char(this->uop_number);

    PackageString = PackageString + " | LOCK:" + get_enum_lock_char(this->lock_type);

    PackageString = PackageString + " | " + get_enum_memory_operation_char(this->initial_memory_operation);
    PackageString = PackageString + " 0x" + utils_t::uint64_to_char(this->initial_memory_address);
    PackageString = PackageString + " Size:" + utils_t::uint64_to_char(this->initial_memory_size);

    PackageString = PackageString + " | BORN:" + utils_t::uint64_to_char(this->born_cycle);

    PackageString = PackageString + " | RQST:" + utils_t::uint32_to_char(this->cache_requested);
    for (uint32_t i = 0; i < sinuca_engine.get_cache_memory_array_size(); i++) {
        PackageString = PackageString + " [" + utils_t::uint32_to_char(this->cache_request_order[i]) + "]";
    }
    return PackageString;
};

//==============================================================================
bool directory_controller_line_t::check_age(container_ptr_directory_controller_line_t *input_array, uint32_t size_array) {
    uint64_t min_cycle = 0;
    if (sinuca_engine.get_global_cycle() > MAX_ALIVE_TIME) {
        min_cycle = sinuca_engine.get_global_cycle() - MAX_ALIVE_TIME;
    }

    for (uint32_t i = 0; i < size_array ; i++) {
        directory_controller_line_t *directory_line = input_array[0][i];
        if (directory_line->born_cycle < min_cycle) {
            return FAIL;
        }
    }
    return OK;
};


//==============================================================================
// directory_controller_t
//==============================================================================
directory_controller_t::directory_controller_t() {
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(sinuca_engine.get_global_line_size()), "Wrong line_size.\n");
    this->mask_addr = ~utils_t::fill_bit(0, utils_t::get_power_of_two(sinuca_engine.get_global_line_size()) - 1);
    this->directory_lines = new container_ptr_directory_controller_line_t;
};

//==============================================================================
directory_controller_t::~directory_controller_t() {
    /// De-Allocate memory to prevent memory leak
    this->directory_lines->clear();
    for (uint32_t i = 0; i < this->directory_lines->size(); i++) {
        WARNING_PRINTF("Directory was not empty.\n")
        directory_controller_line_t *directory_line = this->directory_lines[0][i];
        utils_t::template_delete_variable<directory_controller_line_t>(directory_line);
    }

    utils_t::template_delete_variable<container_ptr_directory_controller_line_t>(this->directory_lines);
    utils_t::template_delete_variable<container_ptr_cache_memory_t>(this->llc_caches);
};

//==============================================================================
void directory_controller_t::allocate() {
    /// Store pointers to all the LLC caches
    this->llc_caches = new container_ptr_cache_memory_t;
    /// Find all LLC
    for (uint32_t i = 0; i < sinuca_engine.get_cache_memory_array_size(); i++) {
        cache_memory_t *cache_memory = sinuca_engine.cache_memory_array[i];
        container_ptr_cache_memory_t *lower_level_cache = cache_memory->get_lower_level_cache();
        /// Found LLC
        if (lower_level_cache->size() == 0) {
            this->llc_caches->push_back(cache_memory);
        }
    }
};

//==============================================================================
void directory_controller_t::clock(uint32_t subcycle) {
    if (subcycle != 0) return;
    DIRECTORY_CTRL_DEBUG_PRINTF("==================== ");
    DIRECTORY_CTRL_DEBUG_PRINTF("====================\n");
    DIRECTORY_CTRL_DEBUG_PRINTF("cycle() \n");
};

//==============================================================================
bool directory_controller_t::receive_package(memory_package_t *package, uint32_t input_port) {
    ERROR_PRINTF("Received package %s into the input_port %u.\n", package->memory_to_string().c_str(), input_port);
    return FAIL;
};

//==============================================================================
package_state_t directory_controller_t::treat_cache_request(uint32_t cache_id, memory_package_t *package) {
    DIRECTORY_CTRL_DEBUG_PRINTF("new_cache_request() cache_id:%u, package:%s\n", cache_id, package->memory_to_string().c_str())
    ERROR_ASSERT_PRINTF(cache_id < sinuca_engine.get_cache_memory_array_size(), "Wrong cache_id.\n")

    /// Get CACHE pointer
    cache_memory_t *cache = sinuca_engine.cache_memory_array[cache_id];
    /// Get CACHE_LINE
    cache_line_t *cache_line = cache->find_line(package->memory_address);

    /// Inspect IS_READ
    bool is_read = this->coherence_is_read(package->memory_operation);
    /// Inspect IS_HIT
    bool is_hit = this->coherence_is_hit(cache_line, package);

    /// ========================================================================
    /// Takes care about the LOCK
    /// ========================================================================
    directory_controller_line_t *directory_line = NULL;
    /// Find an existing directory line.
    for (uint32_t i = 0; i < this->directory_lines->size(); i++) {
        /// Transaction on the same address was found
        if (this->cmp_index_tag(this->directory_lines[0][i]->initial_memory_address, package->memory_address)){
            ///===========================================
            /// Directory Line Refers to this package ?
            if (cache->get_hierarchy_level() != 1 &&                                    /// If NOT Cache L1 (New directory line may be created)
            cache->get_id() != package->id_owner &&                                     /// If NOT This level created the package (New directory line may be created) (Avoid the case when a copy back and prefetch happens together)
            this->directory_lines[0][i]->id_owner == package->id_owner &&               /// If the package Owner is the same
            this->directory_lines[0][i]->opcode_number == package->opcode_number &&
            this->directory_lines[0][i]->uop_number == package->uop_number) {
                directory_line = this->directory_lines[0][i];
                DIRECTORY_CTRL_DEBUG_PRINTF("\t Found Directory Line:%s\n", directory_line->directory_controller_line_to_string().c_str())
                break;
            }

            ///===========================================
            /// Directory Line Refers to other transaction
            else {
                ERROR_ASSERT_PRINTF(directory_lines[0][i]->lock_type != LOCK_FREE, "Found directory with LOCK_FREE\n");

                /// If READ (Need LOCK_FREE/LOCK_READ => LOCK_READ)
                if (is_read) {
                    /// LOCK READ
                    if (directory_lines[0][i]->lock_type == LOCK_READ) {
                        /// May find a directory line from this request
                        continue;
                    }
                    /// LOCK WRITE
                    else {
                        /// Cannot continue right now
                        DIRECTORY_CTRL_DEBUG_PRINTF("\t RETURN UNTREATED (READ found LOCK_W)\n")
                        return PACKAGE_STATE_UNTREATED;
                    }

                }
                /// If WRITE (Need LOCK_FREE => LOCK_WRITE)
                else {
                    /// Cannot continue right now
                    DIRECTORY_CTRL_DEBUG_PRINTF("\t RETURN UNTREATED (WRITE found LOCK_*)\n")
                    return PACKAGE_STATE_UNTREATED;
                }
            }
        }
    }

    /// If L1 or create on this cache
    if (cache->get_hierarchy_level() == 1 || cache->get_id() == package->id_owner) {
        ERROR_ASSERT_PRINTF(directory_line == NULL, "This level REQUEST must not have a directory_line.\n cache_id:%u, package:%s\n", cache->get_id(), package->memory_to_string().c_str())
    }
    else {
        ERROR_ASSERT_PRINTF(directory_line != NULL, "Higher level REQUEST must have a directory_line.\n. cache_id:%u, package:%s\n", cache->get_id(), package->memory_to_string().c_str())
    }


    /// ========================================================================
    /// Takes care about Parallel Requests at the same Cache Level
    /// ========================================================================
    /// Check for parallel requests
    for (uint32_t i = 0; i < this->directory_lines->size(); i++) {
        /// Find Parallel Request
        if (directory_lines[0][i]->cache_request_order[cache_id] != 0 &&
        this->cmp_index_tag(directory_lines[0][i]->initial_memory_address, package->memory_address)) {
            /// No Directory_line yet => create
            if (directory_line == NULL) {
                this->directory_lines->push_back(new directory_controller_line_t());
                directory_line = this->directory_lines->back();

                directory_line->packager(package->id_owner, package->opcode_number, package->opcode_address, package->uop_number,
                                            is_read ? LOCK_READ : LOCK_WRITE,
                                            package->memory_operation, package->memory_address, package->memory_size);
                DIRECTORY_CTRL_DEBUG_PRINTF("\t New Directory Line:%s\n", directory_line->directory_controller_line_to_string().c_str())
            }
            /// Update existing Directory_Line
            directory_line->cache_request_order[cache_id] = ++directory_line->cache_requested;
            DIRECTORY_CTRL_DEBUG_PRINTF("\t Parallel Request:%s\n", directory_line->directory_controller_line_to_string().c_str())
            /// Sit and wait for a answer for the same line
            DIRECTORY_CTRL_DEBUG_PRINTF("\t RETURN WAIT (Parallel Request)\n")
            return PACKAGE_STATE_WAIT;
        }
    }


    /// ========================================================================
    /// Takes care about the CACHE HIT/MISS
    /// ========================================================================
    switch (package->memory_operation) {
        ///=====================================================================
        /// READ and WRITE
        case MEMORY_OPERATION_READ:
        case MEMORY_OPERATION_INST:
        case MEMORY_OPERATION_PREFETCH:
        case MEMORY_OPERATION_WRITE:
            /// Cache Hit
            if (is_hit) {
                /// THIS cache level started the request (PREFETCH)
                if (package->id_owner == cache->get_id()) {
                    /// Update Coherence Status
                    this->coherence_new_operation(cache, cache_line, package, true);
                    /// Erase the answer
                    DIRECTORY_CTRL_DEBUG_PRINTF("\t RETURN FREE (Requester = This)\n")
                    return PACKAGE_STATE_FREE;
                }
                /// Need to send answer to higher level
                else {
                    /// Update Coherence Status
                    this->coherence_new_operation(cache, cache_line, package, true);
                    /// Add Latency
                    if (package->memory_operation == MEMORY_OPERATION_WRITE) {
                        package->ready_cycle = sinuca_engine.get_global_cycle() + cache->get_penalty_write();
                        /// Size of ACK
                        package->memory_size = 1;
                    }
                    else {
                        package->ready_cycle = sinuca_engine.get_global_cycle() + cache->get_penalty_read();
                    }

                    /// Send ANSWER
                    package->is_answer = true;
                    package->id_dst = package->id_src;
                    package->id_src = cache->get_id();
                    DIRECTORY_CTRL_DEBUG_PRINTF("\t RETURN TRANSMIT (Hit)\n")
                    return PACKAGE_STATE_TRANSMIT;
                }
            }
            /// Cache Miss
            else {
                /// Do not have a reserved line
                if (cache_line == NULL) {
                    cache_line = cache->evict_address(package->memory_address);
                    /// Could not evict any line
                    if (cache_line == NULL) {
                        /// Cannot continue right now
                        DIRECTORY_CTRL_DEBUG_PRINTF("\t RETURN UNTREATED (Cache_line == NULL)\n")
                        return PACKAGE_STATE_UNTREATED;
                    }
                }
                /// Already have a reserved line or Found Line to Evict

                /// Need CopyBack ?
                if (!coherence_need_copyback(cache_line)) {
                    /// Add statistics to the cache
                    cache->cache_evict(package->memory_address, false);
                }
                else {
                    if (this->create_cache_copyback(cache, cache_line)) {
                        /// Add statistics to the cache
                        cache->cache_evict(package->memory_address, true);
                    }
                    else {
                        /// Cannot continue right now
                        DIRECTORY_CTRL_DEBUG_PRINTF("\t RETURN UNTREATED (Find_free MSHR == NULL)\n")
                        return PACKAGE_STATE_UNTREATED;
                    }
                }

                /// Found Line to Evict
                /// No Need for CopyBack or CopyBack allocated

                /// Reserve the evicted line for the new address
                cache->change_address(cache_line, package->memory_address);
                /// Coherence Invalidate
                cache->change_status(cache_line, PROTOCOL_STATUS_I);


                /// The request can be treated now !
                /// New Directory_Line + LOCK
                if (directory_line == NULL) {
                    this->directory_lines->push_back(new directory_controller_line_t());
                    directory_line = this->directory_lines->back();

                    directory_line->packager(package->id_owner, package->opcode_number, package->opcode_address, package->uop_number,
                                                is_read ? LOCK_READ : LOCK_WRITE,
                                                package->memory_operation, package->memory_address, package->memory_size);
                    DIRECTORY_CTRL_DEBUG_PRINTF("\t New Directory Line:%s\n", directory_line->directory_controller_line_to_string().c_str())
                }

                /// Update the Directory_Line
                directory_line->cache_request_order[cache_id] = ++directory_line->cache_requested;
                DIRECTORY_CTRL_DEBUG_PRINTF("\t Update Directory Line:%s\n", directory_line->directory_controller_line_to_string().c_str())

                /// Send Request to fill the cache line
                if (package->memory_operation == MEMORY_OPERATION_WRITE) {
                    package->memory_operation = MEMORY_OPERATION_READ;
                }

                /// Check if some HIGHER LEVEL has the cache line Modified
                cache->change_status(cache_line, this->look_higher_levels(cache, package->memory_address, true));


                /// Higher Level Hit
                if (cache_line->status != PROTOCOL_STATUS_I) {
                    /// LATENCY = COPY_BACK + WRITE
                    // TODO(mazalves): Add the copyback latency here !!!
                    package->ready_cycle = sinuca_engine.get_global_cycle() + cache->get_penalty_write();
                    package->is_answer = true;
                    DIRECTORY_CTRL_DEBUG_PRINTF("\t RETURN UNTREATED ANSWER (Found Higher Level)\n")
                    return PACKAGE_STATE_UNTREATED;
                }
                /// Higher Level Miss
                else {
                    /// LATENCY = READ
                    package->ready_cycle = sinuca_engine.get_global_cycle() + cache->get_penalty_read();

                    package->memory_size = sinuca_engine.get_global_line_size();
                    package->id_src = cache->get_id();
                    package->id_dst = this->find_next_obj_id(cache, package->memory_address);
                    DIRECTORY_CTRL_DEBUG_PRINTF("\t RETURN TRANSMIT RQST (Miss)\n")
                    return PACKAGE_STATE_TRANSMIT;
                }
            }
        break;


        ///=====================================================================
        /// COPY-BACK
        case MEMORY_OPERATION_COPYBACK:
            /// COPYBACK from THIS cache => LOWER level
            ERROR_ASSERT_PRINTF(cache->get_id() != package->id_owner, "Copyback should be created using create_cache_copyback.\n")

            /// COPYBACK from HIGHER cache => THIS level
            /// Do not have a reserved line
            if (cache_line == NULL) {
                cache_line = cache->evict_address(package->memory_address);
                /// Could not evict any line
                if (cache_line == NULL) {
                    /// Cannot continue right now
                    DIRECTORY_CTRL_DEBUG_PRINTF("\t RETURN UNTREATED (Cache_line == NULL)\n")
                    return PACKAGE_STATE_UNTREATED;
                }
            }
            /// Already have a reserved line or Found Line to Evict

            /// Need CopyBack ?
            if (!coherence_need_copyback(cache_line)) {
                /// Add statistics to the cache
                cache->cache_evict(package->memory_address, false);
            }
            else {
                if (this->create_cache_copyback(cache, cache_line)) {
                    /// Add statistics to the cache
                    cache->cache_evict(package->memory_address, true);
                }
                else {
                    /// Cannot continue right now
                    DIRECTORY_CTRL_DEBUG_PRINTF("\t RETURN UNTREATED (Find_free MSHR == NULL)\n")
                    return PACKAGE_STATE_UNTREATED;
                }
            }

            /// Found Line to Evict
            /// No Need for CopyBack or CopyBack allocated

            /// Reserve the evicted line for the new address
            cache->change_address(cache_line, package->memory_address);
            /// Coherence Invalidate
            cache->change_status(cache_line, PROTOCOL_STATUS_I);
            /// Update Coherence Status
            this->coherence_new_operation(cache, cache_line, package, true);
            /// Add Latency
            package->ready_cycle = sinuca_engine.get_global_cycle() + cache->get_penalty_write();

            /// Send ANSWER
            package->is_answer = true;
            package->memory_size = 1;
            package->id_dst = package->id_src;
            package->id_src = cache->get_id();
            DIRECTORY_CTRL_DEBUG_PRINTF("\t RETURN TRANSMIT (Hit)\n")
            return PACKAGE_STATE_TRANSMIT;
        break;
    }


    ERROR_PRINTF("Could not treat the cache request\n")
    return PACKAGE_STATE_FREE;
};


//==============================================================================
package_state_t directory_controller_t::treat_cache_answer(uint32_t cache_id, memory_package_t *package) {
    DIRECTORY_CTRL_DEBUG_PRINTF("new_cache_request() cache_id:%u, package:%s\n", cache_id, package->memory_to_string().c_str())
    ERROR_ASSERT_PRINTF(cache_id < sinuca_engine.get_cache_memory_array_size(), "Wrong cache_id.\n")

    /// Get CACHE pointer
    cache_memory_t *cache = sinuca_engine.cache_memory_array[cache_id];
    /// Get CACHE_LINE
    cache_line_t *cache_line = cache->find_line(package->memory_address);
    /// Get CACHE_MSHR
    memory_package_t *cache_mshr_buffer = cache->get_mshr_buffer();
    /// Get CACHE_MSHR_SIZE
    uint32_t cache_mshr_buffer_size = cache->get_mshr_buffer_size();

    directory_controller_line_t *directory_line = NULL;
    int32_t directory_line_number = POSITION_FAIL;
    /// Find the existing directory line.
    for (uint32_t i = 0; i < this->directory_lines->size(); i++) {
        /// Requested Address Found
        if (this->directory_lines[0][i]->id_owner == package->id_owner &&
        this->directory_lines[0][i]->opcode_number == package->opcode_number &&
        this->directory_lines[0][i]->uop_number == package->uop_number &&
        this->cmp_index_tag(directory_lines[0][i]->initial_memory_address, package->memory_address)) {
            directory_line = this->directory_lines[0][i];
            directory_line_number = i;
            break;
        }
    }
    ERROR_ASSERT_PRINTF(directory_line != NULL, "Higher level REQUEST must have a directory_line\n")

    /// ========================================================================
    /// Takes care about Parallel Requests at the same Cache Level
    /// ========================================================================

    /// Wake up all requests waiting in the cache_mshr
    for (uint32_t i = 0; i < cache_mshr_buffer_size; i++) {
        if (cache_mshr_buffer[i].state == PACKAGE_STATE_WAIT &&
        this->cmp_index_tag(cache_mshr_buffer[i].memory_address, package->memory_address)) {
            cache_mshr_buffer[i].is_answer = package->is_answer;
            cache_mshr_buffer[i].memory_size = package->memory_size;
            cache_mshr_buffer[i].id_src = package->id_src;
            cache_mshr_buffer[i].id_dst = package->id_dst;
            cache_mshr_buffer[i].package_untreated(0);
        }
    }

    /// ========================================================================
    /// Takes care about the Coherence Update and Answer
    /// ========================================================================
    switch (package->memory_operation) {
        ///=====================================================================
        /// READ AND WRITE
        case MEMORY_OPERATION_READ:
        case MEMORY_OPERATION_INST:
        case MEMORY_OPERATION_PREFETCH:
        case MEMORY_OPERATION_WRITE:
        case MEMORY_OPERATION_COPYBACK:
                /// THIS cache level started the request
                if (directory_line->id_owner == cache->get_id()) {
                    /// Erase the directory_entry
                    DIRECTORY_CTRL_DEBUG_PRINTF("\t Erasing Directory Line:%s\n", directory_line->directory_controller_line_to_string().c_str())
                    utils_t::template_delete_variable<directory_controller_line_t>(directory_line);
                    directory_line = NULL;
                    this->directory_lines->erase(this->directory_lines->begin() + directory_line_number);
                    /// Update Coherence Status
                    this->coherence_new_operation(cache, cache_line, package, false);
                    /// Erase the answer
                    DIRECTORY_CTRL_DEBUG_PRINTF("\t RETURN FREE (Requester = This)\n")
                    return PACKAGE_STATE_FREE;
                }
                /// Need to send answer to higher level
                else {
                    /// Update existing Directory_Line
                    ERROR_ASSERT_PRINTF(directory_line->cache_request_order[cache_id] == directory_line->cache_requested, "Wrong Cache Request Order\n")
                    directory_line->cache_requested--;
                    directory_line->cache_request_order[cache_id] = 0;
                    DIRECTORY_CTRL_DEBUG_PRINTF("\t Update Directory Line:%s\n", directory_line->directory_controller_line_to_string().c_str())
                    /// This is the first cache requester
                    if (directory_line->cache_requested == 0) {
                        /// Get the DST ID
                        package->id_src = cache->get_id();
                        package->id_dst = directory_line->id_owner;
                        package->memory_operation = directory_line->initial_memory_operation;
                        package->memory_address = directory_line->initial_memory_address;
                        /// Put the correct package size
                        if (package->memory_operation == MEMORY_OPERATION_WRITE || package->memory_operation == MEMORY_OPERATION_COPYBACK) {
                            /// Size of ACK
                            package->memory_size = 1;
                        }
                        else {
                            /// Size of cache line or request
                            package->memory_size = directory_line->initial_memory_size;
                        }
                        /// Erase the directory_entry
                        DIRECTORY_CTRL_DEBUG_PRINTF("\t Erasing Directory Line:%s\n", directory_line->directory_controller_line_to_string().c_str())
                        utils_t::template_delete_variable<directory_controller_line_t>(directory_line);
                        directory_line = NULL;
                        this->directory_lines->erase(this->directory_lines->begin() + directory_line_number);
                        /// Update Coherence Status
                        this->coherence_new_operation(cache, cache_line, package, false);
                        /// Send the answer
                        DIRECTORY_CTRL_DEBUG_PRINTF("\t RETURN TRANSMIT ANS (First Cache Requested)\n")
                        return PACKAGE_STATE_TRANSMIT;
                    }
                    /// This is NOT the first cache requester
                    else {
                        for (uint32_t i = 0; i < sinuca_engine.cache_memory_array_size; i++) {
                            /// Found the previous requester
                            if (directory_line->cache_request_order[i] == directory_line->cache_requested) {
                                /// Get the DST ID
                                package->id_src = cache->get_id();
                                package->id_dst = sinuca_engine.cache_memory_array[i]->get_id();
                                /// Put the correct package size
                                if (package->memory_operation == MEMORY_OPERATION_WRITE || package->memory_operation == MEMORY_OPERATION_COPYBACK) {
                                    /// Size of ACK
                                    package->memory_size = 1;
                                }
                                /// Update Coherence Status
                                this->coherence_new_operation(cache, cache_line, package, false);
                                /// Send the answer
                                DIRECTORY_CTRL_DEBUG_PRINTF("\t RETURN TRANSMIT ANS (NOT First Cache Requested)\n")
                                return PACKAGE_STATE_TRANSMIT;
                            }
                        }
                    }
                }
        break;
    }

    ERROR_PRINTF("Could not treat the cache answer\n")
    return PACKAGE_STATE_FREE;
};


//==============================================================================
/*! This method should be only called if there is no directory lock for the
 * cache line being copyback.
 */
bool directory_controller_t::create_cache_copyback(cache_memory_t *cache, cache_line_t *cache_line) {
    /// Get CACHE_MSHR
    memory_package_t *cache_mshr_buffer = cache->get_mshr_buffer();
    /// Get CACHE_MSHR_SIZE
    uint32_t mshr_buffer_request_reserved_size = cache->get_mshr_buffer_request_reserved_size();
    uint32_t mshr_buffer_copyback_reserved_size = cache->get_mshr_buffer_copyback_reserved_size();

    /// Check for MSHR empty position
    int32_t slot = memory_package_t::find_free(cache_mshr_buffer + mshr_buffer_request_reserved_size, mshr_buffer_copyback_reserved_size);
    if (slot == POSITION_FAIL) {
        cache->add_stat_full_mshr_buffer_copyback();
        return FAIL;
    }
    slot += mshr_buffer_request_reserved_size;
    ///=========================================================================
    /// Allocate CopyBack at the MSHR
    ///=========================================================================
    cache_mshr_buffer[slot].packager(
                cache->get_id(),                        /// Request Owner
                0,                                      /// Opcode. Number
                0,                                      /// Opcode. Address
                0,                                      /// Uop. Number

                cache_line->tag,                        /// Mem. Address
                sinuca_engine.get_global_line_size(),   /// Block Size

                PACKAGE_STATE_TRANSMIT,                 /// Pack. State
                0,                                      /// Ready Cycle

                MEMORY_OPERATION_COPYBACK,              /// Mem. Operation
                false,                                  /// Is Answer

                cache->get_id(),                        /// Src ID
                cache->get_id(),                        /// Dst ID
                NULL,                                   /// *Hops
                0                                       /// Hop Counter
                );
    memory_package_t *package = &cache_mshr_buffer[slot];
    cache->change_status(cache_line, PROTOCOL_STATUS_I);
    cache->change_address(cache_line, 0);

    ///=========================================================================
    /// Allocate CopyBack at the Directory_Line + LOCK
    ///=========================================================================
    this->directory_lines->push_back(new directory_controller_line_t());
    directory_controller_line_t *directory_line = this->directory_lines->back();

    directory_line->packager(package->id_owner, package->opcode_number, package->opcode_address, package->uop_number,
                                LOCK_WRITE,
                                package->memory_operation, package->memory_address, package->memory_size);
    DIRECTORY_CTRL_DEBUG_PRINTF("\t New Directory Line:%s\n", directory_line->directory_controller_line_to_string().c_str())

    /// Update the Directory_Line
    directory_line->cache_request_order[cache->get_cache_id()] = ++directory_line->cache_requested;
    DIRECTORY_CTRL_DEBUG_PRINTF("\t Update Directory Line:%s\n", directory_line->directory_controller_line_to_string().c_str())

    /// Higher Level Copy Back
    package->memory_size = sinuca_engine.get_global_line_size();
    package->id_src = cache->get_id();
    package->id_dst = this->find_next_obj_id(cache, package->memory_address);
    DIRECTORY_CTRL_DEBUG_PRINTF("\t RETURN TRANSMIT RQST (Miss)\n")
    return OK;
};


// =============================================================================
uint32_t directory_controller_t::find_next_obj_id(cache_memory_t *cache_memory, uint64_t memory_address) {
    container_ptr_cache_memory_t *lower_level_cache = cache_memory->get_lower_level_cache();

    /// Find Next Cache Level
    for (uint32_t i = 0; i < lower_level_cache->size(); i++) {
        cache_memory_t *lower_cache = lower_level_cache[0][i];
        if (lower_cache->get_bank(memory_address) == lower_cache->get_bank_number()) {
            return lower_cache->get_id();
        }
    }
    ERROR_ASSERT_PRINTF(lower_level_cache->size() == 0, "Could not find a valid lower_level_cache\n")
    /// Find Next Main Memory
    for (uint32_t i = 0; i < sinuca_engine.main_memory_array_size; i++) {
        if (sinuca_engine.main_memory_array[i]->get_channel(memory_address) == sinuca_engine.main_memory_array[i]->get_channel_number()) {
            return sinuca_engine.main_memory_array[i]->get_id();
        }
    }
    ERROR_PRINTF("Could not find a next_level for the memory address\n")
    return FAIL;
};


// =============================================================================
bool directory_controller_t::is_locked(uint64_t memory_address) {
    /// Check for a lock to the address.
    for (uint32_t i = 0; i < this->directory_lines->size(); i++) {
        /// Same address found
        if (this->cmp_index_tag(directory_lines[0][i]->initial_memory_address, memory_address)) {
            /// Is Locked
            return OK;
        }
    }
    /// Is Un-Locked
    return FAIL;
};


//==============================================================================
protocol_status_t directory_controller_t::look_higher_levels(cache_memory_t *cache_memory, uint64_t memory_address, bool check_llc) {
    ERROR_ASSERT_PRINTF(cache_memory != NULL, "Received a NULL cache_memory\n");

    /// ========================================================================
    /// IF this is the LAST LEVEL CACHE && Want to inspect others LLC
    container_ptr_cache_memory_t *lower_level_cache = cache_memory->get_lower_level_cache();
    if (lower_level_cache->size() == 0 && check_llc == true) {
        /// Iterate over all LLC
        for (uint32_t i = 0; i < this->llc_caches->size(); i++) {
            cache_memory_t *llc = this->llc_caches[0][i];

            /// If bank invalid for the address
            if (llc->get_bank(memory_address) != llc->get_bank_number()) {
                continue;
            }

            /// Propagate Higher
            protocol_status_t return_type = this->look_higher_levels(llc, memory_address, false);

            switch (this->get_coherence_protocol_type()) {
                case COHERENCE_PROTOCOL_MOESI:
                    switch (return_type) {
                        case PROTOCOL_STATUS_M:
                        case PROTOCOL_STATUS_O:
                            return PROTOCOL_STATUS_O;
                        break;

                        case PROTOCOL_STATUS_E:
                        case PROTOCOL_STATUS_S:
                            return PROTOCOL_STATUS_S;
                        break;

                        case PROTOCOL_STATUS_I:
                        break;
                    }
                break;
            }
        }
    }

    /// ========================================================================
    /// IF this is NOT the LLC
    else {
        /// ========================================================================
        /// Check this level
        cache_line_t *this_cache_line = cache_memory->find_line(memory_address);
        if (this_cache_line != NULL) {
            switch (this->get_coherence_protocol_type()) {
                case COHERENCE_PROTOCOL_MOESI:
                    switch (this_cache_line->status) {
                        case PROTOCOL_STATUS_M:
                        case PROTOCOL_STATUS_O:
                            /// This Level stays with a normal copy
                            cache_memory->change_status(this_cache_line, PROTOCOL_STATUS_S);
                            /// Lower level becomes the owner
                            return PROTOCOL_STATUS_O;
                        break;

                        case PROTOCOL_STATUS_E:
                        case PROTOCOL_STATUS_S:
                            return PROTOCOL_STATUS_S;
                        break;

                        case PROTOCOL_STATUS_I:
                        break;
                    }
                break;
            }
        }

        /// ========================================================================
        /// Check on the higher levels
        container_ptr_cache_memory_t *higher_level_cache = cache_memory->get_higher_level_cache();
        for (uint32_t i = 0; i < higher_level_cache->size(); i++) {
            cache_memory_t *higher_cache = higher_level_cache[0][i];
            protocol_status_t return_type = this->look_higher_levels(higher_cache, memory_address, check_llc);

            switch (this->get_coherence_protocol_type()) {
                case COHERENCE_PROTOCOL_MOESI:
                    switch (return_type) {
                        case PROTOCOL_STATUS_M:
                        case PROTOCOL_STATUS_O:
                            switch (this->inclusiveness_type) {
                                case INCLUSIVENESS_NON_INCLUSIVE:
                                    /// Allocate in this level only if line already exist
                                    if (this_cache_line != NULL) {
                                        cache_memory->change_status(this_cache_line, PROTOCOL_STATUS_S);
                                    }
                                    return PROTOCOL_STATUS_O;
                                break;
                            }
                        break;

                        case PROTOCOL_STATUS_E:
                        case PROTOCOL_STATUS_S:
                            return PROTOCOL_STATUS_S;
                        break;

                        case PROTOCOL_STATUS_I:
                        break;
                    }
                break;
            }
        }
    }

    /// No Higher Level Valid Status Found
    return PROTOCOL_STATUS_I;
};


//==============================================================================
bool directory_controller_t::coherence_is_read(memory_operation_t memory_operation) {
    switch (memory_operation) {
        case MEMORY_OPERATION_READ:
        case MEMORY_OPERATION_INST:
        case MEMORY_OPERATION_PREFETCH:
            return OK;
        break;

        case MEMORY_OPERATION_WRITE:
        case MEMORY_OPERATION_COPYBACK:
            return FAIL;
        break;
    }
    ERROR_PRINTF("Wrong memory_operation\n")
    return FAIL;
};

//==============================================================================
bool directory_controller_t::coherence_is_hit(cache_line_t *cache_line,  memory_package_t *package) {
    if (cache_line == NULL){
        return FAIL;
    }
    switch (this->coherence_protocol_type) {
        case COHERENCE_PROTOCOL_MOESI:

            switch (package->memory_operation) {
                case MEMORY_OPERATION_READ:
                case MEMORY_OPERATION_INST:
                case MEMORY_OPERATION_PREFETCH:
                    switch (cache_line->status) {
                        case PROTOCOL_STATUS_M:
                        case PROTOCOL_STATUS_O:
                        case PROTOCOL_STATUS_E:
                        case PROTOCOL_STATUS_S:
                            return OK;
                        break;

                        case PROTOCOL_STATUS_I:
                            return FAIL;
                        break;
                    }
                break;

                case MEMORY_OPERATION_WRITE:
                    switch (cache_line->status) {
                        case PROTOCOL_STATUS_M:
                        case PROTOCOL_STATUS_O:
                        case PROTOCOL_STATUS_E:
                        case PROTOCOL_STATUS_S:
                                return OK;
                        break;

                        case PROTOCOL_STATUS_I:
                                return FAIL;
                        break;
                    }
                break;

                case MEMORY_OPERATION_COPYBACK:
                    switch (cache_line->status) {
                        case PROTOCOL_STATUS_M:
                        case PROTOCOL_STATUS_E:
                        case PROTOCOL_STATUS_O:
                        case PROTOCOL_STATUS_S:
                        case PROTOCOL_STATUS_I:
                            return OK;
                        break;
                    }
                break;

            }
        break;

    }
    ERROR_PRINTF("Invalid protocol status\n");
    return FAIL;
};

//==============================================================================
bool directory_controller_t::coherence_need_copyback(cache_line_t *cache_line) {
    ERROR_ASSERT_PRINTF(cache_line != NULL, "Received a NULL cache_line\n");

    switch (this->coherence_protocol_type) {
        case COHERENCE_PROTOCOL_MOESI:
            switch (cache_line->status) {
                case PROTOCOL_STATUS_M:
                case PROTOCOL_STATUS_O:
                    return OK;
                break;

                case PROTOCOL_STATUS_E:
                case PROTOCOL_STATUS_S:
                case PROTOCOL_STATUS_I:
                    return FAIL;
                break;
            }
        break;
    }
    ERROR_PRINTF("Invalid protocol status\n");
    return FAIL;
};

//==============================================================================
void directory_controller_t::coherence_invalidate_all(cache_memory_t *cache_memory, uint64_t memory_address) {
    /// Get pointers to all cache lines.
    for (uint32_t i = 0; i < sinuca_engine.get_cache_memory_array_size(); i++) {
        /// Cache different to THIS
        if (i != cache_memory->get_cache_id()) {
            cache_line_t *cache_line = sinuca_engine.cache_memory_array[i]->find_line(memory_address);
            if (cache_line != NULL) {
                cache_line->status = PROTOCOL_STATUS_I;
                cache_line->tag = 0;
                /// Cache Statistics
                sinuca_engine.cache_memory_array[i]->cache_invalidate(memory_address, false);
            }
        }
    }
};

//==============================================================================
void directory_controller_t::coherence_new_operation(cache_memory_t *cache, cache_line_t *cache_line,  memory_package_t *package, bool is_hit) {
    ERROR_ASSERT_PRINTF(cache != NULL, "Received a NULL cache_memory\n");

    switch (this->coherence_protocol_type) {
        case COHERENCE_PROTOCOL_MOESI:
            switch (package->memory_operation) {
                case MEMORY_OPERATION_READ:
                case MEMORY_OPERATION_INST:
                case MEMORY_OPERATION_PREFETCH:
                    /// Update the Replacemente Policy information
                    cache->update_last_access(cache_line);
                    if (cache_line->status == PROTOCOL_STATUS_I) {
                        cache_line->status = PROTOCOL_STATUS_S;
                    }
                break;

                case MEMORY_OPERATION_WRITE:
                    /// Update the Replacemente Policy information
                    cache->update_last_access(cache_line);
                    this->coherence_invalidate_all(cache, package->memory_address);
                    cache_line->status = PROTOCOL_STATUS_M;
                break;

                case MEMORY_OPERATION_COPYBACK:
                    ERROR_ASSERT_PRINTF(cache_line == NULL || cache_line->status == PROTOCOL_STATUS_I, "Receiving a Copyback the line should be NULL or INVALID\n")
                    if (is_hit) {
                        ERROR_ASSERT_PRINTF(cache->get_id() != package->id_owner, "CopyBack Hit on the same cache which stated the copyback\n")
                        /// Update the Replacement Policy information
                        cache->update_last_access(cache_line);
                        cache_line->status = PROTOCOL_STATUS_O;
                    }
                break;
            }
        break;
    }


    /// Statistics - HIT
    if (is_hit) {
        cache->cache_hit(package);
        switch (package->memory_operation) {
            case MEMORY_OPERATION_READ:
                this->add_stat_read_hit();
            break;

            case MEMORY_OPERATION_INST:
                this->add_stat_instruction_hit();
            break;

            case MEMORY_OPERATION_PREFETCH:
                this->add_stat_prefetch_hit();
            break;

            case MEMORY_OPERATION_WRITE:
                this->add_stat_write_hit();
            break;

            case MEMORY_OPERATION_COPYBACK:
                this->add_stat_copyback_hit();
            break;
        }
    }

    /// Statistics - MISS
    else {
        cache->cache_miss(package);
        switch (package->memory_operation) {
            case MEMORY_OPERATION_READ:
                this->add_stat_read_miss(package->born_cycle);
            break;

            case MEMORY_OPERATION_INST:
                this->add_stat_instruction_miss(package->born_cycle);
            break;

            case MEMORY_OPERATION_PREFETCH:
                this->add_stat_prefetch_miss(package->born_cycle);
            break;

            case MEMORY_OPERATION_WRITE:
                this->add_stat_write_miss(package->born_cycle);
            break;

            case MEMORY_OPERATION_COPYBACK:
                this->add_stat_copyback_miss(package->born_cycle);
            break;
        }
    }

};



//==============================================================================
void directory_controller_t::print_structures() {
    SINUCA_PRINTF("DIRECTORY_LINE:\n")
    for (uint32_t i = 0; i < this->directory_lines->size(); i++) {
        SINUCA_PRINTF("[%u] %s\n", i, this->directory_lines[0][i]->directory_controller_line_to_string().c_str());
    }
};

// =============================================================================
void directory_controller_t::panic() {
    this->print_structures();
};

//==============================================================================
void directory_controller_t::periodic_check(){
    #ifdef DIRECTORY_CTRL_DEBUG
        DIRECTORY_CTRL_DEBUG_PRINTF("\n");
        this->print_structures();
    #endif
    ERROR_ASSERT_PRINTF(directory_controller_line_t::check_age(this->directory_lines, this->directory_lines->size()) == OK, "Check_age failed.\n");
};

//==============================================================================
// STATISTICS
//==============================================================================
void directory_controller_t::reset_statistics() {

    this->set_stat_instruction_hit(0);
    this->set_stat_read_hit(0);
    this->set_stat_prefetch_hit(0);
    this->set_stat_write_hit(0);
    this->set_stat_copyback_hit(0);

    this->set_stat_instruction_miss(0);
    this->set_stat_read_miss(0);
    this->set_stat_prefetch_miss(0);
    this->set_stat_write_miss(0);
    this->set_stat_copyback_miss(0);

    this->stat_min_instruction_wait_time = MAX_ALIVE_TIME;
    this->stat_max_instruction_wait_time = 0;
    this->stat_acumulated_instruction_wait_time = 0;

    this->stat_min_read_wait_time = MAX_ALIVE_TIME;
    this->stat_max_read_wait_time = 0;
    this->stat_acumulated_read_wait_time = 0;

    this->stat_min_prefetch_wait_time = MAX_ALIVE_TIME;
    this->stat_max_prefetch_wait_time = 0;
    this->stat_acumulated_prefetch_wait_time = 0;

    this->stat_min_write_wait_time = MAX_ALIVE_TIME;
    this->stat_max_write_wait_time = 0;
    this->stat_acumulated_write_wait_time = 0;

    this->stat_min_copyback_wait_time = MAX_ALIVE_TIME;
    this->stat_max_copyback_wait_time = 0;
    this->stat_acumulated_copyback_wait_time = 0;

};

void directory_controller_t::print_statistics() {
    char title[50] = "";
    sprintf(title, "Statistics of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_instruction_hit", stat_instruction_hit);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_read_hit", stat_read_hit);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_prefetch_hit", stat_prefetch_hit);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_write_hit", stat_write_hit);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_copyback_hit", stat_copyback_hit);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_instruction_miss", stat_instruction_miss);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_read_miss", stat_read_miss);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_prefetch_miss", stat_prefetch_miss);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_write_miss", stat_write_miss);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_copyback_miss", stat_copyback_miss);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value_percentage(get_type_component_label(), get_label(), "stat_instruction_miss_percentage",stat_instruction_miss, stat_instruction_miss + stat_instruction_hit);
    sinuca_engine.write_statistics_value_percentage(get_type_component_label(), get_label(), "stat_read_miss_percentage",stat_read_miss, stat_read_miss + stat_read_hit);
    sinuca_engine.write_statistics_value_percentage(get_type_component_label(), get_label(), "stat_prefetch_miss_percentage",stat_prefetch_miss, stat_prefetch_miss + stat_prefetch_hit);
    sinuca_engine.write_statistics_value_percentage(get_type_component_label(), get_label(), "stat_write_miss_percentage",stat_write_miss, stat_write_miss + stat_write_hit);
    sinuca_engine.write_statistics_value_percentage(get_type_component_label(), get_label(), "stat_copyback_miss_percentage",stat_copyback_miss, stat_copyback_miss + stat_copyback_hit);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_min_instruction_wait_time", stat_min_instruction_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_min_read_wait_time", stat_min_read_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_min_prefetch_wait_time", stat_min_prefetch_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_min_write_wait_time", stat_min_write_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_min_copyback_wait_time", stat_min_copyback_wait_time);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_max_instruction_wait_time", stat_max_instruction_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_max_read_wait_time", stat_max_read_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_max_prefetch_wait_time", stat_max_prefetch_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_max_write_wait_time", stat_max_write_wait_time);
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "stat_max_copyback_wait_time", stat_max_copyback_wait_time);

    sinuca_engine.write_statistics_small_separator();
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_instruction_wait_time_ratio",stat_acumulated_instruction_wait_time, stat_instruction_miss);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_read_wait_time_ratio",stat_acumulated_read_wait_time, stat_read_miss);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_prefetch_wait_time_ratio",stat_acumulated_prefetch_wait_time, stat_prefetch_miss);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_write_wait_time_ratio",stat_acumulated_write_wait_time, stat_write_miss);
    sinuca_engine.write_statistics_value_ratio(get_type_component_label(), get_label(), "stat_acumulated_copyback_wait_time_ratio",stat_acumulated_copyback_wait_time, stat_copyback_miss);
};

void directory_controller_t::print_configuration() {
    char title[50] = "";
    sprintf(title, "Configuration of %s", this->get_label());
    sinuca_engine.write_statistics_big_separator();
    sinuca_engine.write_statistics_comments(title);
    sinuca_engine.write_statistics_big_separator();

    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "coherence_protocol_type", get_enum_coherence_protocol_char(coherence_protocol_type));
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "inclusiveness_type", get_enum_inclusiveness_char(inclusiveness_type));
    sinuca_engine.write_statistics_value(get_type_component_label(), get_label(), "mask_addr", utils_t::address_to_binary(this->mask_addr).c_str());
};
