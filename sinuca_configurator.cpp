/// ============================================================================
///
/// Copyright (C) 2010, 2011, 2012
/// Marco Antonio Zanata Alves
///
/// GPPD - Parallel and Distributed Processing Group
/// Universidade Federal do Rio Grande do Sul
///
/// This program is free software; you can redistribute it and/or modify it
/// under the terms of the GNU General Public License as published by the
/// Free Software Foundation; either version 2 of the License, or (at your
/// option) any later version.
///
/// This program is distributed in the hope that it will be useful, but
/// WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
/// General Public License for more details.
///
/// You should have received a copy of the GNU General Public License along
/// with this program; if not, write to the Free Software Foundation, Inc.,
/// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
///
/// ============================================================================
#include "./sinuca.hpp"

#ifdef CONFIGURATOR_DEBUG
    #define CONFIGURATOR_DEBUG_PRINTF(...) SINUCA_PRINTF(__VA_ARGS__);
#else
    #define CONFIGURATOR_DEBUG_PRINTF(...)
#endif
/// ============================================================================

/// ============================================================================
void sinuca_engine_t::initialize() {
    // =========================================================================
    // Read the file. If there is an error, report it and exit.
    // =========================================================================
    libconfig::Config cfg;
    cfg.setIncludeDir(this->arg_configuration_path);

    try {
        cfg.readFile(this->arg_configuration_file_name);
    }
    catch(libconfig::FileIOException &fioex) {
        ERROR_PRINTF("I/O error while reading file.\n")
    }
    catch(libconfig::ParseException &pex) {
        ERROR_PRINTF("Parse error at %s on line %d: %s\n", pex.getFile(), pex.getLine(), pex.getError());
    }

    libconfig::Setting &cfg_root = cfg.getRoot();

    /// Check if all required components exist
    for (uint32_t i = 0 ; i < COMPONENT_NUMBER; i++) {
        if(! cfg_root.exists(get_enum_component_char((component_t)i))) {
            ERROR_PRINTF("Required COMPONENT missing:\"%s\"\n", get_enum_component_char((component_t)i));
        }
    }

    SINUCA_PRINTF("\n");
    this->initialize_processor();
    this->initialize_cache_memory();
    this->initialize_memory_controller();
    this->initialize_interconnection_router();

    this->initialize_directory_controller();
    this->initialize_interconnection_controller();
    SINUCA_PRINTF("\n");
    CONFIGURATOR_DEBUG_PRINTF("Finished components instantiation\n");

    /// ========================================================================
    /// Connect the pointers to the interconnection interface
    this->set_interconnection_interface_array_size( this->get_processor_array_size() +
                                                    this->get_cache_memory_array_size() +
                                                    this->get_memory_controller_array_size() +
                                                    this->get_interconnection_router_array_size()
                                                    );
    this->interconnection_interface_array = utils_t::template_allocate_initialize_array<interconnection_interface_t*>(this->interconnection_interface_array_size, NULL);


    uint32_t total_components = 0;
    for (uint32_t i = 0; i < this->get_processor_array_size(); i++) {
        this->interconnection_interface_array[total_components] = this->processor_array[i];
        this->interconnection_interface_array[total_components]->set_id(total_components);
        total_components++;
    }
    for (uint32_t i = 0; i < this->get_cache_memory_array_size(); i++) {
        this->interconnection_interface_array[total_components] = this->cache_memory_array[i];
        this->interconnection_interface_array[total_components]->set_id(total_components);
        total_components++;
    }
    for (uint32_t i = 0; i < this->get_memory_controller_array_size(); i++) {
        this->interconnection_interface_array[total_components] = this->memory_controller_array[i];
        this->interconnection_interface_array[total_components]->set_id(total_components);
        total_components++;
    }
    for (uint32_t i = 0; i < this->get_interconnection_router_array_size(); i++) {
        this->interconnection_interface_array[total_components] = this->interconnection_router_array[i];
        this->interconnection_interface_array[total_components]->set_id(total_components);
        total_components++;
    }
    ERROR_ASSERT_PRINTF(total_components == this->get_interconnection_interface_array_size(), "Wrong number of components (%u).\n", total_components)

    /// ========================================================================
    /// Call the allocate() from all components
    for (uint32_t i = 0; i < this->get_interconnection_interface_array_size(); i++) {
        CONFIGURATOR_DEBUG_PRINTF("Allocating %s\n", this->interconnection_interface_array[i]->get_label());
        this->interconnection_interface_array[i]->allocate();
        this->interconnection_interface_array[i]->allocate_base();
    }
    this->set_is_simulation_allocated(true);
    CONFIGURATOR_DEBUG_PRINTF("Finished allocation\n");

    /// ========================================================================
    /// Connects:   processor <=> cache_memory
    ///             directory <=> cache memory
    ///             lower cache level <=> higher cache level
    this->make_connections();
    CONFIGURATOR_DEBUG_PRINTF("Finished make connections\n");

    /// Check if all the ports has connections and are connected.
    for (uint32_t i = 0; i < get_interconnection_interface_array_size(); i++) {
        interconnection_interface_t *obj = interconnection_interface_array[i];
        for (uint32_t j = 0; j < obj->get_max_ports(); j++) {
            ERROR_ASSERT_PRINTF(obj->get_interface_output_component(j) !=  NULL, "Port %u of component %s is not connected\n", j, obj->get_label());
        }
    }

    /// Print the Cache connections IF DEBUG MODE
    #ifdef CONFIGURATOR_DEBUG
        std::string tree = "";
        for (uint32_t i = 0; i < get_cache_memory_array_size(); i++) {
            cache_memory_t *cache_memory = cache_memory_array[i];
            container_ptr_cache_memory_t *lower_level_cache = cache_memory->get_lower_level_cache();
            /// Found the Lowest Level Cache (Closest to the Main Memory)
            if (lower_level_cache->empty()) {
                tree += utils_t::connections_pretty(cache_memory, 0);
            }
        }
    #endif
    CONFIGURATOR_DEBUG_PRINTF("CONNECTIONS:\n%s\n", tree.c_str())

    this->directory_controller->allocate();
    this->interconnection_controller->allocate();
};

/// ============================================================================
void sinuca_engine_t::initialize_processor() {
    libconfig::Config cfg;
    cfg.setIncludeDir(this->arg_configuration_path);
    cfg.readFile(this->arg_configuration_file_name);

    libconfig::Setting &cfg_root = cfg.getRoot();
    libconfig::Setting &cfg_processor_list = cfg_root["PROCESSOR"];
    SINUCA_PRINTF("PROCESSORS:%d\n", cfg_processor_list.getLength());

    this->set_processor_array_size(cfg_processor_list.getLength());
    this->processor_array = utils_t::template_allocate_initialize_array<processor_t*>(this->processor_array_size, NULL);

    /// ========================================================================
    /// Required PROCESSOR Parameters
    /// ========================================================================
    for (int32_t i = 0; i < cfg_processor_list.getLength(); i++) {
        this->processor_array[i] = new processor_t;

        container_ptr_const_char_t processor_parameters;
        container_ptr_const_char_t branch_predictor_parameters;

        libconfig::Setting &cfg_processor = cfg_processor_list[i];
        /// ====================================================================
        /// PROCESSOR PARAMETERS
        /// ====================================================================
        try {
            this->processor_array[i]->set_core_id(i);

            processor_parameters.push_back("LABEL");
            this->processor_array[i]->set_label( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("INTERCONNECTION_LATENCY");
            this->processor_array[i]->set_interconnection_latency( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("INTERCONNECTION_WIDTH");
            this->processor_array[i]->set_interconnection_width( cfg_processor[ processor_parameters.back() ] );

            /// ================================================================
            /// Pipeline Latency
            /// ================================================================
            processor_parameters.push_back("STAGE_FETCH_CYCLES");
            this->processor_array[i]->set_stage_fetch_cycles( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("STAGE_DECODE_CYCLES");
            this->processor_array[i]->set_stage_decode_cycles( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("STAGE_RENAME_CYCLES");
            this->processor_array[i]->set_stage_rename_cycles( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("STAGE_DISPATCH_CYCLES");
            this->processor_array[i]->set_stage_dispatch_cycles( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("STAGE_EXECUTION_CYCLES");
            this->processor_array[i]->set_stage_execution_cycles( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("STAGE_COMMIT_CYCLES");
            this->processor_array[i]->set_stage_commit_cycles( cfg_processor[ processor_parameters.back() ] );

            /// ================================================================
            /// Pipeline Width
            /// ================================================================
            processor_parameters.push_back("STAGE_FETCH_WIDTH");
            this->processor_array[i]->set_stage_fetch_width( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("STAGE_DECODE_WIDTH");
            this->processor_array[i]->set_stage_decode_width( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("STAGE_RENAME_WIDTH");
            this->processor_array[i]->set_stage_rename_width( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("STAGE_DISPATCH_WIDTH");
            this->processor_array[i]->set_stage_dispatch_width( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("STAGE_EXECUTION_WIDTH");
            this->processor_array[i]->set_stage_execution_width( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("STAGE_COMMIT_WIDTH");
            this->processor_array[i]->set_stage_commit_width( cfg_processor[ processor_parameters.back() ] );

            /// ================================================================
            /// Buffers
            /// ================================================================

            processor_parameters.push_back("FETCH_BUFFER_SIZE");
            this->processor_array[i]->set_fetch_buffer_size( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("DECODE_BUFFER_SIZE");
            this->processor_array[i]->set_decode_buffer_size( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("REORDER_BUFFER_SIZE");
            this->processor_array[i]->set_reorder_buffer_size( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("UNIFIED_RESERVATION_STATION_WINDOW_SIZE");
            this->processor_array[i]->set_unified_reservation_station_window_size( cfg_processor[ processor_parameters.back() ] );

            /// ================================================================
            /// Integer Funcional Units
            /// ================================================================
            processor_parameters.push_back("NUMBER_FU_INT_ALU");
            this->processor_array[i]->set_number_fu_int_alu( cfg_processor[ processor_parameters.back() ] );
            processor_parameters.push_back("LATENCY_FU_INT_ALU");
            this->processor_array[i]->set_latency_fu_int_alu( cfg_processor[ processor_parameters.back() ] );
            processor_parameters.push_back("WAIT_BETWEEN_FU_INT_ALU");
            this->processor_array[i]->set_wait_between_fu_int_alu( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("NUMBER_FU_INT_MUL");
            this->processor_array[i]->set_number_fu_int_mul( cfg_processor[ processor_parameters.back() ] );
            processor_parameters.push_back("LATENCY_FU_INT_MUL");
            this->processor_array[i]->set_latency_fu_int_mul( cfg_processor[ processor_parameters.back() ] );
            processor_parameters.push_back("WAIT_BETWEEN_FU_INT_MUL");
            this->processor_array[i]->set_wait_between_fu_int_mul( cfg_processor[ processor_parameters.back() ] );


            processor_parameters.push_back("NUMBER_FU_INT_DIV");
            this->processor_array[i]->set_number_fu_int_div( cfg_processor[ processor_parameters.back() ] );
            processor_parameters.push_back("LATENCY_FU_INT_DIV");
            this->processor_array[i]->set_latency_fu_int_div( cfg_processor[ processor_parameters.back() ] );
            processor_parameters.push_back("WAIT_BETWEEN_FU_INT_DIV");
            this->processor_array[i]->set_wait_between_fu_int_div( cfg_processor[ processor_parameters.back() ] );

            /// ================================================================
            /// Floating Point Funcional Units
            /// ================================================================
            processor_parameters.push_back("NUMBER_FU_FP_ALU");
            this->processor_array[i]->set_number_fu_fp_alu( cfg_processor[ processor_parameters.back() ] );
            processor_parameters.push_back("LATENCY_FU_FP_ALU");
            this->processor_array[i]->set_latency_fu_fp_alu( cfg_processor[ processor_parameters.back() ] );
            processor_parameters.push_back("WAIT_BETWEEN_FU_FP_ALU");
            this->processor_array[i]->set_wait_between_fu_fp_alu( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("NUMBER_FU_FP_MUL");
            this->processor_array[i]->set_number_fu_fp_mul( cfg_processor[ processor_parameters.back() ] );
            processor_parameters.push_back("LATENCY_FU_FP_MUL");
            this->processor_array[i]->set_latency_fu_fp_mul( cfg_processor[ processor_parameters.back() ] );
            processor_parameters.push_back("WAIT_BETWEEN_FU_FP_MUL");
            this->processor_array[i]->set_wait_between_fu_fp_mul( cfg_processor[ processor_parameters.back() ] );


            processor_parameters.push_back("NUMBER_FU_FP_DIV");
            this->processor_array[i]->set_number_fu_fp_div( cfg_processor[ processor_parameters.back() ] );
            processor_parameters.push_back("LATENCY_FU_FP_DIV");
            this->processor_array[i]->set_latency_fu_fp_div( cfg_processor[ processor_parameters.back() ] );
            processor_parameters.push_back("WAIT_BETWEEN_FU_FP_DIV");
            this->processor_array[i]->set_wait_between_fu_fp_div( cfg_processor[ processor_parameters.back() ] );

            /// ================================================================
            /// Memory Funcional Units
            /// ================================================================
            processor_parameters.push_back("NUMBER_FU_MEM_LOAD");
            this->processor_array[i]->set_number_fu_mem_load( cfg_processor[ processor_parameters.back() ] );
            processor_parameters.push_back("LATENCY_FU_MEM_LOAD");
            this->processor_array[i]->set_latency_fu_mem_load( cfg_processor[ processor_parameters.back() ] );
            processor_parameters.push_back("WAIT_BETWEEN_FU_MEM_LOAD");
            this->processor_array[i]->set_wait_between_fu_mem_load( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("NUMBER_FU_MEM_STORE");
            this->processor_array[i]->set_number_fu_mem_store( cfg_processor[ processor_parameters.back() ] );
            processor_parameters.push_back("LATENCY_FU_MEM_STORE");
            this->processor_array[i]->set_latency_fu_mem_store( cfg_processor[ processor_parameters.back() ] );
            processor_parameters.push_back("WAIT_BETWEEN_FU_MEM_STORE");
            this->processor_array[i]->set_wait_between_fu_mem_store( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("MEMORY_ORDER_BUFFER_READ_SIZE");
            this->processor_array[i]->set_memory_order_buffer_read_size( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("MEMORY_ORDER_BUFFER_WRITE_SIZE");
            this->processor_array[i]->set_memory_order_buffer_write_size( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("DISAMBIGUATION_TYPE");
            if (strcasecmp(cfg_processor[ processor_parameters.back() ], "PERFECT") ==  0) {
                this->processor_array[i]->set_disambiguation_type(DISAMBIGUATION_PERFECT);
            }
            else if (strcasecmp(cfg_processor[ processor_parameters.back() ], "DISABLE") ==  0) {
                this->processor_array[i]->set_disambiguation_type(DISAMBIGUATION_DISABLE);
            }
            else {
                ERROR_PRINTF("PROCESSOR %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_processor[ processor_parameters.back() ].c_str(), processor_parameters.back());
            }

            processor_parameters.push_back("DISAMBIGUATION_BLOCK_SIZE");
            this->processor_array[i]->set_disambiguation_block_size( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("REGISTER_FORWARD_LATENCY");
            this->processor_array[i]->set_register_forward_latency( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("SOLVE_ADDRESS_TO_ADDRESS");
            this->processor_array[i]->set_solve_address_to_address( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("FETCH_BLOCK_SIZE");
            this->processor_array[i]->set_fetch_block_size( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("WAIT_WRITE_COMPLETE");
            this->processor_array[i]->set_wait_write_complete( cfg_processor[ processor_parameters.back() ] );


            processor_parameters.push_back("BRANCH_PER_FETCH");
            this->processor_array[i]->set_branch_per_fetch( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("BRANCH_FLUSH_LATENCY");
            this->processor_array[i]->set_branch_flush_latency( cfg_processor[ processor_parameters.back() ] );

            processor_parameters.push_back("INFLIGHT_BRANCHES_SIZE");
            this->processor_array[i]->set_inflight_branches_size( cfg_processor[ processor_parameters.back() ] );


            processor_parameters.push_back("CONNECTED_DATA_CACHE");
            processor_parameters.push_back("CONNECTED_INST_CACHE");

            this->processor_array[i]->set_max_ports(PROCESSOR_PORT_NUMBER);

            /// ================================================================
            /// Required BRANCH PREDICTOR Parameters
            /// ================================================================
            processor_parameters.push_back("BRANCH_PREDICTOR");
            libconfig::Setting &cfg_branch_predictor = cfg_processor_list[i][processor_parameters.back()];

            branch_predictor_parameters.push_back("TYPE");
            if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "TWO_LEVEL_GAG") ==  0) {
                this->processor_array[i]->branch_predictor = new branch_predictor_two_level_gag_t;
                branch_predictor_two_level_gag_t *branch_predictor_ptr = static_cast<branch_predictor_two_level_gag_t*>(this->processor_array[i]->branch_predictor);

                branch_predictor_ptr->set_branch_predictor_type(BRANCH_PREDICTOR_TWO_LEVEL_GAG);

                branch_predictor_parameters.push_back("BTB_LINE_NUMBER");
                branch_predictor_ptr->set_btb_line_number(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("BTB_ASSOCIATIVITY");
                branch_predictor_ptr->set_btb_associativity(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("BTB_REPLACEMENT_POLICY");
                if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "FIFO") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_FIFO);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "LRF") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_LRF);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "LRU") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_LRU);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "RANDOM") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_RANDOM);
                }
                else {
                    ERROR_PRINTF("PROCESSOR %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_branch_predictor[ branch_predictor_parameters.back() ].c_str(), branch_predictor_parameters.back());
                }

                branch_predictor_parameters.push_back("GPHT_LINE_NUMBER");
                branch_predictor_ptr->set_gpht_line_number(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("GPHT_INDEX_HASH");
                if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_ONLY") ==  0) {
                    branch_predictor_ptr->set_gpht_index_hash(HASH_FUNCTION_INPUT1_ONLY);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT2_ONLY") ==  0) {
                    branch_predictor_ptr->set_gpht_index_hash(HASH_FUNCTION_INPUT2_ONLY);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "XOR_SIMPLE") ==  0) {
                    branch_predictor_ptr->set_gpht_index_hash(HASH_FUNCTION_XOR_SIMPLE);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_2BITS") ==  0) {
                    branch_predictor_ptr->set_gpht_index_hash(HASH_FUNCTION_INPUT1_2BITS);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_4BITS") ==  0) {
                    branch_predictor_ptr->set_gpht_index_hash(HASH_FUNCTION_INPUT1_4BITS);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_8BITS") ==  0) {
                    branch_predictor_ptr->set_gpht_index_hash(HASH_FUNCTION_INPUT1_8BITS);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_16BITS") ==  0) {
                    branch_predictor_ptr->set_gpht_index_hash(HASH_FUNCTION_INPUT1_16BITS);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_32BITS") ==  0) {
                    branch_predictor_ptr->set_gpht_index_hash(HASH_FUNCTION_INPUT1_32BITS);
                }
                else {
                    ERROR_PRINTF("PROCESSOR %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_branch_predictor[ branch_predictor_parameters.back() ].c_str(), branch_predictor_parameters.back());
                }

                branch_predictor_parameters.push_back("FSM_BITS");
                branch_predictor_ptr->set_fsm_bits(cfg_branch_predictor[ branch_predictor_parameters.back() ]);
            }
            else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "TWO_LEVEL_GAS") ==  0) {
                this->processor_array[i]->branch_predictor = new branch_predictor_two_level_gas_t;
                branch_predictor_two_level_gas_t *branch_predictor_ptr = static_cast<branch_predictor_two_level_gas_t*>(this->processor_array[i]->branch_predictor);

                branch_predictor_ptr->set_branch_predictor_type(BRANCH_PREDICTOR_TWO_LEVEL_GAS);

                branch_predictor_parameters.push_back("BTB_LINE_NUMBER");
                branch_predictor_ptr->set_btb_line_number(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("BTB_ASSOCIATIVITY");
                branch_predictor_ptr->set_btb_associativity(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("BTB_REPLACEMENT_POLICY");
                if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "FIFO") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_FIFO);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "LRF") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_LRF);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "LRU") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_LRU);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "RANDOM") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_RANDOM);
                }
                else {
                    ERROR_PRINTF("PROCESSOR %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_branch_predictor[ branch_predictor_parameters.back() ].c_str(), branch_predictor_parameters.back());
                }

                branch_predictor_parameters.push_back("SPHT_LINE_NUMBER");
                branch_predictor_ptr->set_spht_line_number(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("SPHT_SET_NUMBER");
                branch_predictor_ptr->set_spht_set_number(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("SPHT_INDEX_HASH");
                if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_ONLY") ==  0) {
                    branch_predictor_ptr->set_spht_index_hash(HASH_FUNCTION_INPUT1_ONLY);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT2_ONLY") ==  0) {
                    branch_predictor_ptr->set_spht_index_hash(HASH_FUNCTION_INPUT2_ONLY);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "XOR_SIMPLE") ==  0) {
                    branch_predictor_ptr->set_spht_index_hash(HASH_FUNCTION_XOR_SIMPLE);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_2BITS") ==  0) {
                    branch_predictor_ptr->set_spht_index_hash(HASH_FUNCTION_INPUT1_2BITS);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_4BITS") ==  0) {
                    branch_predictor_ptr->set_spht_index_hash(HASH_FUNCTION_INPUT1_4BITS);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_8BITS") ==  0) {
                    branch_predictor_ptr->set_spht_index_hash(HASH_FUNCTION_INPUT1_8BITS);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_16BITS") ==  0) {
                    branch_predictor_ptr->set_spht_index_hash(HASH_FUNCTION_INPUT1_16BITS);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_32BITS") ==  0) {
                    branch_predictor_ptr->set_spht_index_hash(HASH_FUNCTION_INPUT1_32BITS);
                }
                else {
                    ERROR_PRINTF("PROCESSOR %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_branch_predictor[ branch_predictor_parameters.back() ].c_str(), branch_predictor_parameters.back());
                }

                branch_predictor_parameters.push_back("FSM_BITS");
                branch_predictor_ptr->set_fsm_bits(cfg_branch_predictor[ branch_predictor_parameters.back() ]);
            }
            else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "TWO_LEVEL_PAG") ==  0) {
                this->processor_array[i]->branch_predictor = new branch_predictor_two_level_pag_t;
                branch_predictor_two_level_pag_t *branch_predictor_ptr = static_cast<branch_predictor_two_level_pag_t*>(this->processor_array[i]->branch_predictor);

                branch_predictor_ptr->set_branch_predictor_type(BRANCH_PREDICTOR_TWO_LEVEL_PAG);

                branch_predictor_parameters.push_back("BTB_LINE_NUMBER");
                branch_predictor_ptr->set_btb_line_number(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("BTB_ASSOCIATIVITY");
                branch_predictor_ptr->set_btb_associativity(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("BTB_REPLACEMENT_POLICY");
                if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "FIFO") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_FIFO);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "LRF") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_LRF);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "LRU") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_LRU);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "RANDOM") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_RANDOM);
                }
                else {
                    ERROR_PRINTF("PROCESSOR %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_branch_predictor[ branch_predictor_parameters.back() ].c_str(), branch_predictor_parameters.back());
                }

                branch_predictor_parameters.push_back("PBHT_LINE_NUMBER");
                branch_predictor_ptr->set_pbht_line_number(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("PBHT_SIGNATURE_BITS");
                branch_predictor_ptr->set_pbht_signature_bits(cfg_branch_predictor[ branch_predictor_parameters.back() ]);


                branch_predictor_parameters.push_back("GPHT_LINE_NUMBER");
                branch_predictor_ptr->set_gpht_line_number(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("GPHT_INDEX_HASH");
                if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_ONLY") ==  0) {
                    branch_predictor_ptr->set_gpht_index_hash(HASH_FUNCTION_INPUT1_ONLY);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT2_ONLY") ==  0) {
                    branch_predictor_ptr->set_gpht_index_hash(HASH_FUNCTION_INPUT2_ONLY);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "XOR_SIMPLE") ==  0) {
                    branch_predictor_ptr->set_gpht_index_hash(HASH_FUNCTION_XOR_SIMPLE);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_2BITS") ==  0) {
                    branch_predictor_ptr->set_gpht_index_hash(HASH_FUNCTION_INPUT1_2BITS);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_4BITS") ==  0) {
                    branch_predictor_ptr->set_gpht_index_hash(HASH_FUNCTION_INPUT1_4BITS);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_8BITS") ==  0) {
                    branch_predictor_ptr->set_gpht_index_hash(HASH_FUNCTION_INPUT1_8BITS);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_16BITS") ==  0) {
                    branch_predictor_ptr->set_gpht_index_hash(HASH_FUNCTION_INPUT1_16BITS);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_32BITS") ==  0) {
                    branch_predictor_ptr->set_gpht_index_hash(HASH_FUNCTION_INPUT1_32BITS);
                }
                else {
                    ERROR_PRINTF("PROCESSOR %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_branch_predictor[ branch_predictor_parameters.back() ].c_str(), branch_predictor_parameters.back());
                }

                branch_predictor_parameters.push_back("FSM_BITS");
                branch_predictor_ptr->set_fsm_bits(cfg_branch_predictor[ branch_predictor_parameters.back() ]);
            }
            else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "TWO_LEVEL_PAS") ==  0) {
                this->processor_array[i]->branch_predictor = new branch_predictor_two_level_pas_t;
                branch_predictor_two_level_pas_t *branch_predictor_ptr = static_cast<branch_predictor_two_level_pas_t*>(this->processor_array[i]->branch_predictor);

                branch_predictor_ptr->set_branch_predictor_type(BRANCH_PREDICTOR_TWO_LEVEL_PAS);

                branch_predictor_parameters.push_back("BTB_LINE_NUMBER");
                branch_predictor_ptr->set_btb_line_number(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("BTB_ASSOCIATIVITY");
                branch_predictor_ptr->set_btb_associativity(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("BTB_REPLACEMENT_POLICY");
                if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "FIFO") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_FIFO);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "LRF") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_LRF);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "LRU") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_LRU);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "RANDOM") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_RANDOM);
                }
                else {
                    ERROR_PRINTF("PROCESSOR %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_branch_predictor[ branch_predictor_parameters.back() ].c_str(), branch_predictor_parameters.back());
                }

                branch_predictor_parameters.push_back("PBHT_LINE_NUMBER");
                branch_predictor_ptr->set_pbht_line_number(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("PBHT_SIGNATURE_BITS");
                branch_predictor_ptr->set_pbht_signature_bits(cfg_branch_predictor[ branch_predictor_parameters.back() ]);


                branch_predictor_parameters.push_back("SPHT_LINE_NUMBER");
                branch_predictor_ptr->set_spht_line_number(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("SPHT_SET_NUMBER");
                branch_predictor_ptr->set_spht_set_number(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("SPHT_INDEX_HASH");
                if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_ONLY") ==  0) {
                    branch_predictor_ptr->set_spht_index_hash(HASH_FUNCTION_INPUT1_ONLY);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT2_ONLY") ==  0) {
                    branch_predictor_ptr->set_spht_index_hash(HASH_FUNCTION_INPUT2_ONLY);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "XOR_SIMPLE") ==  0) {
                    branch_predictor_ptr->set_spht_index_hash(HASH_FUNCTION_XOR_SIMPLE);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_2BITS") ==  0) {
                    branch_predictor_ptr->set_spht_index_hash(HASH_FUNCTION_INPUT1_2BITS);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_4BITS") ==  0) {
                    branch_predictor_ptr->set_spht_index_hash(HASH_FUNCTION_INPUT1_4BITS);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_8BITS") ==  0) {
                    branch_predictor_ptr->set_spht_index_hash(HASH_FUNCTION_INPUT1_8BITS);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_16BITS") ==  0) {
                    branch_predictor_ptr->set_spht_index_hash(HASH_FUNCTION_INPUT1_16BITS);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "INPUT1_32BITS") ==  0) {
                    branch_predictor_ptr->set_spht_index_hash(HASH_FUNCTION_INPUT1_32BITS);
                }
                else {
                    ERROR_PRINTF("PROCESSOR %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_branch_predictor[ branch_predictor_parameters.back() ].c_str(), branch_predictor_parameters.back());
                }

                branch_predictor_parameters.push_back("FSM_BITS");
                branch_predictor_ptr->set_fsm_bits(cfg_branch_predictor[ branch_predictor_parameters.back() ]);
            }
            else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "BI_MODAL") ==  0) {
                this->processor_array[i]->branch_predictor = new branch_predictor_bi_modal_t;
                branch_predictor_bi_modal_t *branch_predictor_ptr = static_cast<branch_predictor_bi_modal_t*>(this->processor_array[i]->branch_predictor);

                branch_predictor_ptr->set_branch_predictor_type(BRANCH_PREDICTOR_BI_MODAL);

                branch_predictor_parameters.push_back("BTB_LINE_NUMBER");
                branch_predictor_ptr->set_btb_line_number(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("BTB_ASSOCIATIVITY");
                branch_predictor_ptr->set_btb_associativity(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("BTB_REPLACEMENT_POLICY");
                if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "FIFO") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_FIFO);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "LRF") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_LRF);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "LRU") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_LRU);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "RANDOM") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_RANDOM);
                }
                else {
                    ERROR_PRINTF("PROCESSOR %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_branch_predictor[ branch_predictor_parameters.back() ].c_str(), branch_predictor_parameters.back());
                }

                branch_predictor_parameters.push_back("BHT_LINE_NUMBER");
                branch_predictor_ptr->set_bht_line_number(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("FSM_BITS");
                branch_predictor_ptr->set_fsm_bits(cfg_branch_predictor[ branch_predictor_parameters.back() ]);
            }
            else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "STATIC_TAKEN") ==  0) {
                this->processor_array[i]->branch_predictor = new branch_predictor_static_taken_t;
                branch_predictor_static_taken_t *branch_predictor_ptr = static_cast<branch_predictor_static_taken_t*>(this->processor_array[i]->branch_predictor);

                branch_predictor_ptr->set_branch_predictor_type(BRANCH_PREDICTOR_STATIC_TAKEN);

                branch_predictor_parameters.push_back("BTB_LINE_NUMBER");
                branch_predictor_ptr->set_btb_line_number(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("BTB_ASSOCIATIVITY");
                branch_predictor_ptr->set_btb_associativity(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("BTB_REPLACEMENT_POLICY");
                if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "FIFO") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_FIFO);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "LRF") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_LRF);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "LRU") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_LRU);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "RANDOM") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_RANDOM);
                }
                else {
                    ERROR_PRINTF("PROCESSOR %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_branch_predictor[ branch_predictor_parameters.back() ].c_str(), branch_predictor_parameters.back());
                }
            }
            else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "PERFECT") ==  0) {
                this->processor_array[i]->branch_predictor = new branch_predictor_perfect_t;
                branch_predictor_perfect_t *branch_predictor_ptr = static_cast<branch_predictor_perfect_t*>(this->processor_array[i]->branch_predictor);

                branch_predictor_ptr->set_branch_predictor_type(BRANCH_PREDICTOR_PERFECT);

                branch_predictor_parameters.push_back("BTB_LINE_NUMBER");
                branch_predictor_ptr->set_btb_line_number(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("BTB_ASSOCIATIVITY");
                branch_predictor_ptr->set_btb_associativity(cfg_branch_predictor[ branch_predictor_parameters.back() ]);

                branch_predictor_parameters.push_back("BTB_REPLACEMENT_POLICY");
                if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "FIFO") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_FIFO);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "LRF") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_LRF);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "LRU") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_LRU);
                }
                else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "RANDOM") ==  0) {
                    branch_predictor_ptr->set_btb_replacement_policy(REPLACEMENT_RANDOM);
                }
                else {
                    ERROR_PRINTF("PROCESSOR %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_branch_predictor[ branch_predictor_parameters.back() ].c_str(), branch_predictor_parameters.back());
                }
            }
            else if (strcasecmp(cfg_branch_predictor[ branch_predictor_parameters.back() ], "DISABLE") ==  0) {
                this->processor_array[i]->branch_predictor = new branch_predictor_disable_t;
                branch_predictor_disable_t *branch_predictor_ptr = static_cast<branch_predictor_disable_t*>(this->processor_array[i]->branch_predictor);

                branch_predictor_ptr->set_branch_predictor_type(BRANCH_PREDICTOR_DISABLE);

            }

            else {
                ERROR_PRINTF("PROCESSOR %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_branch_predictor[ branch_predictor_parameters.back() ].c_str(), branch_predictor_parameters.back());
            }

            /// ================================================================
            /// Check if any PROCESSOR non-required parameters exist
            for (int32_t j = 0 ; j < cfg_processor.getLength(); j++) {
                bool is_required = false;
                for (uint32_t k = 0 ; k < processor_parameters.size(); k++) {
                    if (strcmp(cfg_processor[j].getName(), processor_parameters[k]) == 0) {
                        is_required = true;
                        break;
                    }
                }
                ERROR_ASSERT_PRINTF(is_required, "PROCESSOR %d %s has PARAMETER not required: \"%s\"\n", i, this->processor_array[i]->get_label(), cfg_processor[j].getName());
            }

            /// Check if any BRANCH PREDICTOR non-required parameters exist
            for (int32_t j = 0 ; j < cfg_branch_predictor.getLength(); j++) {
                bool is_required = false;
                for (uint32_t k = 0 ; k < branch_predictor_parameters.size(); k++) {
                    if (strcmp(cfg_branch_predictor[j].getName(), branch_predictor_parameters[k]) == 0) {
                        is_required = true;
                        break;
                    }
                }
                ERROR_ASSERT_PRINTF(is_required, "PROCESSOR %d %s BRANCH PREDICTOR has PARAMETER not required: \"%s\"\n", i, this->processor_array[i]->get_label(), cfg_branch_predictor[j].getName());
            }
        }
        catch(libconfig::SettingNotFoundException &nfex) {
            ERROR_PRINTF(" PROCESSOR %d %s has required PARAMETER missing: \"%s\" \"%s\"\n", i, this->processor_array[i]->get_label(), processor_parameters.back(), branch_predictor_parameters.empty() ? "": branch_predictor_parameters.back());
        }
        catch(libconfig::SettingTypeException &tex) {
            ERROR_PRINTF(" PROCESSOR %d %s has PARAMETER wrong type: \"%s\" \"%s\"\n", i, this->processor_array[i]->get_label(), processor_parameters.back(), branch_predictor_parameters.empty() ? "": branch_predictor_parameters.back());
        }
    }
};


/// ============================================================================
void sinuca_engine_t::initialize_cache_memory() {
    libconfig::Config cfg;
    cfg.setIncludeDir(this->arg_configuration_path);
    cfg.readFile(this->arg_configuration_file_name);

    libconfig::Setting &cfg_root = cfg.getRoot();
    libconfig::Setting &cfg_cache_memory_list = cfg_root["CACHE_MEMORY"];
    SINUCA_PRINTF("CACHE_MEMORIES:%d\n", cfg_cache_memory_list.getLength());

    this->set_cache_memory_array_size(cfg_cache_memory_list.getLength());
    this->cache_memory_array = utils_t::template_allocate_initialize_array<cache_memory_t*>(this->cache_memory_array_size, NULL);

    /// ========================================================================
    /// Required CACHE MEMORY Parameters
    /// ========================================================================
    for (int32_t i = 0; i < cfg_cache_memory_list.getLength(); i++) {
        this->cache_memory_array[i] = new cache_memory_t;

        container_ptr_const_char_t cache_memory_parameters;
        container_ptr_const_char_t prefetcher_parameters;
        container_ptr_const_char_t line_usage_predictor_parameters;

        libconfig::Setting &cfg_cache_memory = cfg_cache_memory_list[i];

        /// ====================================================================
        /// CACHE_MEMORY PARAMETERS
        /// ====================================================================
        try {
            this->cache_memory_array[i]->set_cache_id(i);

            cache_memory_parameters.push_back("LABEL");
            this->cache_memory_array[i]->set_label( cfg_cache_memory[ cache_memory_parameters.back() ] );

            cache_memory_parameters.push_back("INTERCONNECTION_LATENCY");
            this->cache_memory_array[i]->set_interconnection_latency( cfg_cache_memory[ cache_memory_parameters.back() ] );

            cache_memory_parameters.push_back("INTERCONNECTION_WIDTH");
            this->cache_memory_array[i]->set_interconnection_width( cfg_cache_memory[ cache_memory_parameters.back() ] );

            cache_memory_parameters.push_back("HIERARCHY_LEVEL");
            this->cache_memory_array[i]->set_hierarchy_level( cfg_cache_memory[ cache_memory_parameters.back() ] );

            cache_memory_parameters.push_back("PENALTY_READ");
            this->cache_memory_array[i]->set_penalty_read( cfg_cache_memory[ cache_memory_parameters.back() ] );

            cache_memory_parameters.push_back("PENALTY_WRITE");
            this->cache_memory_array[i]->set_penalty_write( cfg_cache_memory[ cache_memory_parameters.back() ] );

            cache_memory_parameters.push_back("LINE_NUMBER");
            this->cache_memory_array[i]->set_line_number( cfg_cache_memory[ cache_memory_parameters.back() ] );

            cache_memory_parameters.push_back("LINE_SIZE");
            this->cache_memory_array[i]->set_line_size( cfg_cache_memory[ cache_memory_parameters.back() ] );
            this->set_global_line_size(this->cache_memory_array[i]->get_line_size());

            cache_memory_parameters.push_back("ASSOCIATIVITY");
            this->cache_memory_array[i]->set_associativity( cfg_cache_memory[ cache_memory_parameters.back() ] );

            cache_memory_parameters.push_back("MSHR_BUFFER_REQUEST_RESERVED_SIZE");
            this->cache_memory_array[i]->set_mshr_buffer_request_reserved_size( cfg_cache_memory[ cache_memory_parameters.back() ] );

            cache_memory_parameters.push_back("MSHR_BUFFER_WRITEBACK_RESERVED_SIZE");
            this->cache_memory_array[i]->set_mshr_buffer_writeback_reserved_size( cfg_cache_memory[ cache_memory_parameters.back() ] );

            cache_memory_parameters.push_back("MSHR_BUFFER_PREFETCH_RESERVED_SIZE");
            this->cache_memory_array[i]->set_mshr_buffer_prefetch_reserved_size( cfg_cache_memory[ cache_memory_parameters.back() ] );


            cache_memory_parameters.push_back("MSHR_REQUEST_DIFFERENT_LINES_SIZE");
            this->cache_memory_array[i]->set_mshr_request_different_lines_size( cfg_cache_memory[ cache_memory_parameters.back() ] );

            cache_memory_parameters.push_back("MSHR_REQUEST_TOKEN_WINDOW_SIZE");
            this->cache_memory_array[i]->set_mshr_request_token_window_size( cfg_cache_memory[ cache_memory_parameters.back() ] );


            cache_memory_parameters.push_back("REPLACEMENT_POLICY");
            if (strcasecmp(cfg_cache_memory[ cache_memory_parameters.back() ], "FIFO") ==  0) {
                this->cache_memory_array[i]->set_replacement_policy(REPLACEMENT_FIFO);
            }
            else if (strcasecmp(cfg_cache_memory[ cache_memory_parameters.back() ], "LRF") ==  0) {
                this->cache_memory_array[i]->set_replacement_policy(REPLACEMENT_LRF);
            }
            else if (strcasecmp(cfg_cache_memory[ cache_memory_parameters.back() ], "LRU") ==  0) {
                this->cache_memory_array[i]->set_replacement_policy(REPLACEMENT_LRU);
            }
            else if (strcasecmp(cfg_cache_memory[ cache_memory_parameters.back() ], "INVALID_OR_LRU") ==  0) {
                this->cache_memory_array[i]->set_replacement_policy(REPLACEMENT_INVALID_OR_LRU);
            }
            else if (strcasecmp(cfg_cache_memory[ cache_memory_parameters.back() ], "DEAD_OR_LRU") ==  0) {
                this->cache_memory_array[i]->set_replacement_policy(REPLACEMENT_DEAD_OR_LRU);
            }
            else if (strcasecmp(cfg_cache_memory[ cache_memory_parameters.back() ], "RANDOM") ==  0) {
                this->cache_memory_array[i]->set_replacement_policy(REPLACEMENT_RANDOM);
            }
            else {
                ERROR_PRINTF("CACHE MEMORY %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_cache_memory[ cache_memory_parameters.back() ].c_str(), cache_memory_parameters.back());
            }

            cache_memory_parameters.push_back("BANK_NUMBER");
            this->cache_memory_array[i]->set_bank_number( cfg_cache_memory[ cache_memory_parameters.back() ] );

            cache_memory_parameters.push_back("TOTAL_BANKS");
            this->cache_memory_array[i]->set_total_banks( cfg_cache_memory[ cache_memory_parameters.back() ] );

            cache_memory_parameters.push_back("ADDRESS_MASK");
            if (strcasecmp(cfg_cache_memory[ cache_memory_parameters.back() ], "TAG_INDEX_OFFSET") ==  0) {
                this->cache_memory_array[i]->set_address_mask_type(CACHE_MASK_TAG_INDEX_OFFSET);
            }
            else if (strcasecmp(cfg_cache_memory[ cache_memory_parameters.back() ], "TAG_INDEX_BANK_OFFSET") ==  0) {
                this->cache_memory_array[i]->set_address_mask_type(CACHE_MASK_TAG_INDEX_BANK_OFFSET);
            }
            else if (strcasecmp(cfg_cache_memory[ cache_memory_parameters.back() ], "TAG_BANK_INDEX_OFFSET") ==  0) {
                this->cache_memory_array[i]->set_address_mask_type(CACHE_MASK_TAG_BANK_INDEX_OFFSET);
            }
            else {
                ERROR_PRINTF("CACHE MEMORY %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_cache_memory[ cache_memory_parameters.back() ].c_str(), cache_memory_parameters.back());
            }

            cache_memory_parameters.push_back("LOWER_LEVEL_CACHE");

            cache_memory_parameters.push_back("MAX_PORTS");
            this->cache_memory_array[i]->set_max_ports( cfg_cache_memory[ cache_memory_parameters.back() ] );

            /// ================================================================
            /// Required PREFETCHER Parameters
            /// ================================================================
            cache_memory_parameters.push_back("PREFETCHER");
            libconfig::Setting &cfg_prefetcher = cfg_cache_memory_list[i][cache_memory_parameters.back()];


            prefetcher_parameters.push_back("TYPE");
            if (strcasecmp(cfg_prefetcher[ prefetcher_parameters.back() ], "STRIDE") ==  0) {
                this->cache_memory_array[i]->prefetcher = new prefetch_stride_t;
                prefetch_stride_t *prefetcher_ptr = static_cast<prefetch_stride_t*>(this->cache_memory_array[i]->prefetcher);

                prefetcher_ptr->set_prefetcher_type(PREFETCHER_STRIDE);

                prefetcher_parameters.push_back("STRIDE_TABLE_SIZE");
                prefetcher_ptr->set_stride_table_size( cfg_prefetcher[ prefetcher_parameters.back() ] );

                prefetcher_parameters.push_back("PREFETCH_DEGREE");
                prefetcher_ptr->set_prefetch_degree( cfg_prefetcher[ prefetcher_parameters.back() ] );

            }
            else if (strcasecmp(cfg_prefetcher[ prefetcher_parameters.back() ], "DISABLE") ==  0) {
                this->cache_memory_array[i]->prefetcher = new prefetch_disable_t;
                prefetch_disable_t *prefetcher_ptr = static_cast<prefetch_disable_t*>(this->cache_memory_array[i]->prefetcher);

                prefetcher_ptr->set_prefetcher_type(PREFETCHER_DISABLE);
            }
            else if (strcasecmp(cfg_prefetcher[ prefetcher_parameters.back() ], "STREAM") ==  0){
                this->cache_memory_array[i]->prefetcher = new prefetch_stream_t;
                prefetch_stream_t *prefetcher_ptr = static_cast<prefetch_stream_t*>(this->cache_memory_array[i]->prefetcher);

                prefetcher_ptr->set_prefetcher_type(PREFETCHER_STREAM);

                prefetcher_parameters.push_back("STREAM_TABLE_SIZE");
                prefetcher_ptr->set_stream_table_size( cfg_prefetcher[ prefetcher_parameters.back() ] );

                prefetcher_parameters.push_back("PREFETCH_DISTANCE");
                prefetcher_ptr->set_prefetch_distance( cfg_prefetcher[ prefetcher_parameters.back() ] );

                prefetcher_parameters.push_back("PREFETCH_DEGREE");
                prefetcher_ptr->set_prefetch_degree( cfg_prefetcher[ prefetcher_parameters.back() ] );

                prefetcher_parameters.push_back("SEARCH_DISTANCE");
                prefetcher_ptr->set_search_distance( cfg_prefetcher[ prefetcher_parameters.back() ] );

                prefetcher_parameters.push_back("LIFETIME_CYCLES");
                prefetcher_ptr->set_lifetime_cycles( cfg_prefetcher[ prefetcher_parameters.back() ] );

            }
            else {
                ERROR_PRINTF("CACHE MEMORY %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_prefetcher[ prefetcher_parameters.back() ].c_str(), prefetcher_parameters.back());
            }

            prefetcher_parameters.push_back("FULL_BUFFER");
            if (strcasecmp(cfg_prefetcher[ prefetcher_parameters.back() ], "OVERRIDE") ==  0) {
                this->cache_memory_array[i]->prefetcher->set_full_buffer_type(FULL_BUFFER_OVERRIDE);
            }
            else if (strcasecmp(cfg_prefetcher[ prefetcher_parameters.back() ], "STOP") ==  0) {
                this->cache_memory_array[i]->prefetcher->set_full_buffer_type(FULL_BUFFER_STOP);
            }
            else {
                ERROR_PRINTF("CACHE MEMORY %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_prefetcher[ prefetcher_parameters.back() ].c_str(), prefetcher_parameters.back());
            }

            prefetcher_parameters.push_back("REQUEST_BUFFER_SIZE");
            this->cache_memory_array[i]->prefetcher->set_request_buffer_size( cfg_prefetcher[ prefetcher_parameters.back() ] );

            /// ================================================================
            /// Required LINE_USAGE_PREDICTOR Parameters
            /// ================================================================
            cache_memory_parameters.push_back("LINE_USAGE_PREDICTOR");
            libconfig::Setting &cfg_line_usage_predictor = cfg_cache_memory_list[i][cache_memory_parameters.back()];

            line_usage_predictor_parameters.push_back("TYPE");
            if (strcasecmp(cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ], "DSBP") ==  0) {
                this->cache_memory_array[i]->line_usage_predictor = new line_usage_predictor_dsbp_t;
                line_usage_predictor_dsbp_t *line_usage_predictor_ptr = static_cast<line_usage_predictor_dsbp_t*>(this->cache_memory_array[i]->line_usage_predictor);

                line_usage_predictor_ptr->set_line_usage_predictor_type(LINE_USAGE_PREDICTOR_POLICY_DSBP);

                line_usage_predictor_parameters.push_back("BYTES_PER_SUBBLOCK");
                line_usage_predictor_ptr->set_bytes_per_subblock( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );

                line_usage_predictor_parameters.push_back("USAGE_COUNTER_BITS_READ");
                line_usage_predictor_ptr->set_access_counter_bits_read( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );


                line_usage_predictor_parameters.push_back("EARLY_EVICTION");
                line_usage_predictor_ptr->set_early_eviction( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );

                line_usage_predictor_parameters.push_back("EARLY_WRITEBACK");
                line_usage_predictor_ptr->set_early_writeback( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );

                line_usage_predictor_parameters.push_back("TURNOFF_DEAD_LINES");
                line_usage_predictor_ptr->set_turnoff_dead_lines( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );


                /// Metadata
                line_usage_predictor_parameters.push_back("METADATA_LINE_NUMBER");
                line_usage_predictor_ptr->set_metadata_line_number( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );
                ERROR_ASSERT_PRINTF(this->cache_memory_array[i]->get_line_number() == line_usage_predictor_ptr->get_metadata_line_number(),
                                    "CACHE MEMORY %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_prefetcher[ line_usage_predictor_parameters.back() ].c_str(), line_usage_predictor_parameters.back());

                line_usage_predictor_parameters.push_back("METADATA_ASSOCIATIVITY");
                line_usage_predictor_ptr->set_metadata_associativity( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );
                ERROR_ASSERT_PRINTF(this->cache_memory_array[i]->get_associativity() == line_usage_predictor_ptr->get_metadata_associativity(),
                                    "CACHE MEMORY %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_prefetcher[ line_usage_predictor_parameters.back() ].c_str(), line_usage_predictor_parameters.back());

                /// PHT
                line_usage_predictor_parameters.push_back("PHT_LINE_NUMBER");
                line_usage_predictor_ptr->set_pht_line_number( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );

                line_usage_predictor_parameters.push_back("PHT_ASSOCIATIVITY");
                line_usage_predictor_ptr->set_pht_associativity( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );

                line_usage_predictor_parameters.push_back("PHT_REPLACEMENT_POLICY");
                if (strcasecmp(cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ], "FIFO") ==  0) {
                    line_usage_predictor_ptr->set_pht_replacement_policy(REPLACEMENT_FIFO);
                }
                else if (strcasecmp(cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ], "LRF") ==  0) {
                    line_usage_predictor_ptr->set_pht_replacement_policy(REPLACEMENT_LRF);
                }
                else if (strcasecmp(cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ], "LRU") ==  0) {
                    line_usage_predictor_ptr->set_pht_replacement_policy(REPLACEMENT_LRU);
                }
                else if (strcasecmp(cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ], "RANDOM") ==  0) {
                    line_usage_predictor_ptr->set_pht_replacement_policy(REPLACEMENT_RANDOM);
                }
                else {
                    ERROR_PRINTF("CACHE MEMORY %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ].c_str(), line_usage_predictor_parameters.back());
                }
            }
            else if (strcasecmp(cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ], "DSBP_ORACLE") ==  0) {
                this->cache_memory_array[i]->line_usage_predictor = new line_usage_predictor_dsbp_oracle_t;
                line_usage_predictor_dsbp_oracle_t *line_usage_predictor_ptr = static_cast<line_usage_predictor_dsbp_oracle_t*>(this->cache_memory_array[i]->line_usage_predictor);

                line_usage_predictor_ptr->set_line_usage_predictor_type(LINE_USAGE_PREDICTOR_POLICY_DSBP_ORACLE);

                line_usage_predictor_parameters.push_back("BYTES_PER_SUBBLOCK");
                line_usage_predictor_ptr->set_bytes_per_subblock( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );

                line_usage_predictor_parameters.push_back("BRING_WHOLE_LINE");
                line_usage_predictor_ptr->set_bring_whole_line( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );


                /// Metadata
                line_usage_predictor_parameters.push_back("METADATA_LINE_NUMBER");
                line_usage_predictor_ptr->set_metadata_line_number( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );
                ERROR_ASSERT_PRINTF(this->cache_memory_array[i]->get_line_number() == line_usage_predictor_ptr->get_metadata_line_number(),
                                    "CACHE MEMORY %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_prefetcher[ line_usage_predictor_parameters.back() ].c_str(), line_usage_predictor_parameters.back());

                line_usage_predictor_parameters.push_back("METADATA_ASSOCIATIVITY");
                line_usage_predictor_ptr->set_metadata_associativity( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );
                ERROR_ASSERT_PRINTF(this->cache_memory_array[i]->get_associativity() == line_usage_predictor_ptr->get_metadata_associativity(),
                                    "CACHE MEMORY %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_prefetcher[ line_usage_predictor_parameters.back() ].c_str(), line_usage_predictor_parameters.back());

            }
            else if (strcasecmp(cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ], "DEWP") ==  0) {
                this->cache_memory_array[i]->line_usage_predictor = new line_usage_predictor_dewp_t;
                line_usage_predictor_dewp_t *line_usage_predictor_ptr = static_cast<line_usage_predictor_dewp_t*>(this->cache_memory_array[i]->line_usage_predictor);

                line_usage_predictor_ptr->set_line_usage_predictor_type(LINE_USAGE_PREDICTOR_POLICY_DEWP);


                /// Metadata
                line_usage_predictor_parameters.push_back("METADATA_LINE_NUMBER");
                line_usage_predictor_ptr->set_metadata_line_number( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );
                ERROR_ASSERT_PRINTF(this->cache_memory_array[i]->get_line_number() == line_usage_predictor_ptr->get_metadata_line_number(),
                                    "CACHE MEMORY %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_prefetcher[ line_usage_predictor_parameters.back() ].c_str(), line_usage_predictor_parameters.back());

                line_usage_predictor_parameters.push_back("METADATA_ASSOCIATIVITY");
                line_usage_predictor_ptr->set_metadata_associativity( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );
                ERROR_ASSERT_PRINTF(this->cache_memory_array[i]->get_associativity() == line_usage_predictor_ptr->get_metadata_associativity(),
                                    "CACHE MEMORY %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_prefetcher[ line_usage_predictor_parameters.back() ].c_str(), line_usage_predictor_parameters.back());

                line_usage_predictor_parameters.push_back("USAGE_COUNTER_BITS_READ");
                line_usage_predictor_ptr->set_access_counter_bits_read( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );

                line_usage_predictor_parameters.push_back("USAGE_COUNTER_BITS_WRITEBACK");
                line_usage_predictor_ptr->set_access_counter_bits_writeback( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );

                line_usage_predictor_parameters.push_back("EARLY_EVICTION");
                line_usage_predictor_ptr->set_early_eviction( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );

                line_usage_predictor_parameters.push_back("EARLY_WRITEBACK");
                line_usage_predictor_ptr->set_early_writeback( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );

                line_usage_predictor_parameters.push_back("TURNOFF_DEAD_LINES");
                line_usage_predictor_ptr->set_turnoff_dead_lines( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );

                /// AHT
                line_usage_predictor_parameters.push_back("AHT_LINE_NUMBER");
                line_usage_predictor_ptr->set_aht_line_number( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );

                line_usage_predictor_parameters.push_back("AHT_ASSOCIATIVITY");
                line_usage_predictor_ptr->set_aht_associativity( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );

                line_usage_predictor_parameters.push_back("AHT_REPLACEMENT_POLICY");
                if (strcasecmp(cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ], "FIFO") ==  0) {
                    line_usage_predictor_ptr->set_aht_replacement_policy(REPLACEMENT_FIFO);
                }
                else if (strcasecmp(cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ], "LRF") ==  0) {
                    line_usage_predictor_ptr->set_aht_replacement_policy(REPLACEMENT_LRF);
                }
                else if (strcasecmp(cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ], "LRU") ==  0) {
                    line_usage_predictor_ptr->set_aht_replacement_policy(REPLACEMENT_LRU);
                }
                else if (strcasecmp(cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ], "RANDOM") ==  0) {
                    line_usage_predictor_ptr->set_aht_replacement_policy(REPLACEMENT_RANDOM);
                }
                else {
                    ERROR_PRINTF("CACHE MEMORY %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ].c_str(), line_usage_predictor_parameters.back());
                }
            }
            else if (strcasecmp(cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ], "DEWP_ORACLE") ==  0) {
                this->cache_memory_array[i]->line_usage_predictor = new line_usage_predictor_dewp_oracle_t;
                line_usage_predictor_dewp_oracle_t *line_usage_predictor_ptr = static_cast<line_usage_predictor_dewp_oracle_t*>(this->cache_memory_array[i]->line_usage_predictor);

                line_usage_predictor_ptr->set_line_usage_predictor_type(LINE_USAGE_PREDICTOR_POLICY_DEWP_ORACLE);

                /// Metadata
                line_usage_predictor_parameters.push_back("METADATA_LINE_NUMBER");
                line_usage_predictor_ptr->set_metadata_line_number( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );
                ERROR_ASSERT_PRINTF(this->cache_memory_array[i]->get_line_number() == line_usage_predictor_ptr->get_metadata_line_number(),
                                    "CACHE MEMORY %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_prefetcher[ line_usage_predictor_parameters.back() ].c_str(), line_usage_predictor_parameters.back());

                line_usage_predictor_parameters.push_back("METADATA_ASSOCIATIVITY");
                line_usage_predictor_ptr->set_metadata_associativity( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );
                ERROR_ASSERT_PRINTF(this->cache_memory_array[i]->get_associativity() == line_usage_predictor_ptr->get_metadata_associativity(),
                                    "CACHE MEMORY %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_prefetcher[ line_usage_predictor_parameters.back() ].c_str(), line_usage_predictor_parameters.back());

            }
            else if (strcasecmp(cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ], "SKEWED") ==  0) {
                this->cache_memory_array[i]->line_usage_predictor = new line_usage_predictor_skewed_t;
                line_usage_predictor_skewed_t *line_usage_predictor_ptr = static_cast<line_usage_predictor_skewed_t*>(this->cache_memory_array[i]->line_usage_predictor);

                line_usage_predictor_ptr->set_line_usage_predictor_type(LINE_USAGE_PREDICTOR_POLICY_SKEWED);

                line_usage_predictor_parameters.push_back("EARLY_EVICTION");
                line_usage_predictor_ptr->set_early_eviction( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );

                line_usage_predictor_parameters.push_back("EARLY_WRITEBACK");
                line_usage_predictor_ptr->set_early_writeback( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );

                line_usage_predictor_parameters.push_back("TURNOFF_DEAD_LINES");
                line_usage_predictor_ptr->set_turnoff_dead_lines( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );

                /// Skewed Table
                line_usage_predictor_parameters.push_back("SKEWED_TABLE_LINE_NUMBER");
                line_usage_predictor_ptr->set_skewed_table_line_number( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );

                line_usage_predictor_parameters.push_back("FSM_BITS");
                line_usage_predictor_ptr->set_fsm_bits( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );

                /// Metadata
                line_usage_predictor_parameters.push_back("METADATA_LINE_NUMBER");
                line_usage_predictor_ptr->set_metadata_line_number( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );
                ERROR_ASSERT_PRINTF(this->cache_memory_array[i]->get_line_number() == line_usage_predictor_ptr->get_metadata_line_number(),
                                    "CACHE MEMORY %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_prefetcher[ line_usage_predictor_parameters.back() ].c_str(), line_usage_predictor_parameters.back());

                line_usage_predictor_parameters.push_back("METADATA_ASSOCIATIVITY");
                line_usage_predictor_ptr->set_metadata_associativity( cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ] );
                ERROR_ASSERT_PRINTF(this->cache_memory_array[i]->get_associativity() == line_usage_predictor_ptr->get_metadata_associativity(),
                                    "CACHE MEMORY %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_prefetcher[ line_usage_predictor_parameters.back() ].c_str(), line_usage_predictor_parameters.back());

            }            else if (strcasecmp(cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ], "DISABLE") ==  0) {
                this->cache_memory_array[i]->line_usage_predictor = new line_usage_predictor_disable_t;
                line_usage_predictor_disable_t *line_usage_predictor_ptr = static_cast<line_usage_predictor_disable_t*>(this->cache_memory_array[i]->line_usage_predictor);

                line_usage_predictor_ptr->set_line_usage_predictor_type(LINE_USAGE_PREDICTOR_POLICY_DISABLE);
            }
            else {
                ERROR_PRINTF("CACHE MEMORY %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_line_usage_predictor[ line_usage_predictor_parameters.back() ].c_str(), line_usage_predictor_parameters.back());
            }

            /// ================================================================
            /// Check if any CACHE_MEMORY non-required parameters exist
            for (int32_t j = 0 ; j < cfg_cache_memory.getLength(); j++) {
                bool is_required = false;
                for (uint32_t k = 0 ; k < cache_memory_parameters.size(); k++) {
                    if (strcmp(cfg_cache_memory[j].getName(), cache_memory_parameters[k]) == 0) {
                        is_required = true;
                        break;
                    }
                }
                ERROR_ASSERT_PRINTF(is_required, "CACHE_MEMORY %d %s has PARAMETER not required: \"%s\"\n", i, this->cache_memory_array[i]->get_label(), cfg_cache_memory[j].getName());
            }

            /// Check if any PREFETCHER non-required parameters exist
            for (int32_t j = 0 ; j < cfg_prefetcher.getLength(); j++) {
                bool is_required = false;
                for (uint32_t k = 0 ; k < prefetcher_parameters.size(); k++) {
                    if (strcmp(cfg_prefetcher[j].getName(), prefetcher_parameters[k]) == 0) {
                        is_required = true;
                        break;
                    }
                }
                ERROR_ASSERT_PRINTF(is_required, "CACHE_MEMORY %d %s PREFETCHER has PARAMETER not required: \"%s\"\n", i, this->cache_memory_array[i]->get_label(), cfg_prefetcher[j].getName());
            }

            /// Check if any LINE_USAGE_PREDICTOR non-required parameters exist
            for (int32_t j = 0 ; j < cfg_line_usage_predictor.getLength(); j++) {
                bool is_required = false;
                for (uint32_t k = 0 ; k < line_usage_predictor_parameters.size(); k++) {
                    if (strcmp(cfg_line_usage_predictor[j].getName(), line_usage_predictor_parameters[k]) == 0) {
                        is_required = true;
                        break;
                    }
                }
                ERROR_ASSERT_PRINTF(is_required, "CACHE_MEMORY %d %s LINE_USAGE_PREDICTOR has PARAMETER not required: \"%s\"\n", i, this->cache_memory_array[i]->get_label(), cfg_line_usage_predictor[j].getName());
            }

        }
        catch(libconfig::SettingNotFoundException &nfex) {
            /// Cache Memory Parameter
            if (prefetcher_parameters.empty() && line_usage_predictor_parameters.empty()) {
                ERROR_PRINTF(" CACHE_MEMORY %d %s has required PARAMETER missing: \"%s\"\n", i, this->cache_memory_array[i]->get_label(), cache_memory_parameters.back());
            }
            /// Prefetcher Parameter
            else if (prefetcher_parameters.empty() == false && line_usage_predictor_parameters.empty()) {
                ERROR_PRINTF(" CACHE_MEMORY-Prefetcher %d %s has required PARAMETER missing: \"%s\" \"%s\" \n", i, this->cache_memory_array[i]->get_label(), cache_memory_parameters.back(), prefetcher_parameters.back());
            }
            /// Line_Usage_Predictor Parameter
            else {
                ERROR_PRINTF(" CACHE_MEMORY-Line_Usage_Predictor %d %s has required PARAMETER missing: \"%s\" \"%s\" \n", i, this->cache_memory_array[i]->get_label(), cache_memory_parameters.back(), line_usage_predictor_parameters.back());
            }
        }
        catch(libconfig::SettingTypeException &tex) {
            /// Cache Memory Parameter
            if (prefetcher_parameters.empty() && line_usage_predictor_parameters.empty()) {
                ERROR_PRINTF(" CACHE_MEMORY %d %s has required PARAMETER wrong type: \"%s\"\n", i, this->cache_memory_array[i]->get_label(), cache_memory_parameters.back());
            }
            /// Prefetcher Parameter
            else if (prefetcher_parameters.empty() == false && line_usage_predictor_parameters.empty()) {
                ERROR_PRINTF(" CACHE_MEMORY-Prefetcher %d %s has required PARAMETER wrong type: \"%s\" \"%s\" \n", i, this->cache_memory_array[i]->get_label(), cache_memory_parameters.back(), prefetcher_parameters.back());
            }
            /// Line_Usage_Predictor Parameter
            else {
                ERROR_PRINTF(" CACHE_MEMORY-Line_Usage_Predictor %d %s has required PARAMETER wrong type: \"%s\" \"%s\" \n", i, this->cache_memory_array[i]->get_label(), cache_memory_parameters.back(), line_usage_predictor_parameters.back());
            }
        }
    }
};


/// ============================================================================
void sinuca_engine_t::initialize_memory_controller() {
    libconfig::Config cfg;
    cfg.setIncludeDir(this->arg_configuration_path);
    cfg.readFile(this->arg_configuration_file_name);

    libconfig::Setting &cfg_root = cfg.getRoot();
    libconfig::Setting &cfg_memory_controller_list = cfg_root["MEMORY_CONTROLLER"];
    SINUCA_PRINTF("MAIN_MEMORIES:%d\n", cfg_memory_controller_list.getLength());

    this->set_memory_controller_array_size(cfg_memory_controller_list.getLength());
    this->memory_controller_array = utils_t::template_allocate_initialize_array<memory_controller_t*>(this->memory_controller_array_size, NULL);

    /// ========================================================================
    /// Required MEMORY_CONTROLLER Parameters
    /// ========================================================================
    for (int32_t i = 0; i < cfg_memory_controller_list.getLength(); i++) {
        this->memory_controller_array[i] = new memory_controller_t;

        container_ptr_const_char_t memory_controller_parameters;
        libconfig::Setting &cfg_memory_controller = cfg_memory_controller_list[i];

        /// ====================================================================
        /// MEMORY_CONTROLLER PARAMETERS
        /// ====================================================================
        try {
            memory_controller_parameters.push_back("LABEL");
            this->memory_controller_array[i]->set_label( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("INTERCONNECTION_LATENCY");
            this->memory_controller_array[i]->set_interconnection_latency( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("INTERCONNECTION_WIDTH");
            this->memory_controller_array[i]->set_interconnection_width( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("ADDRESS_MASK");
            if (strcasecmp(cfg_memory_controller[ memory_controller_parameters.back() ], "ROW_BANK_CHANNEL_CTRL_COLUMN") ==  0) {
                this->memory_controller_array[i]->set_address_mask_type(MEMORY_CONTROLLER_MASK_ROW_BANK_CHANNEL_CTRL_COLUMN);
            }
            else if (strcasecmp(cfg_memory_controller[ memory_controller_parameters.back() ], "ROW_BANK_CHANNEL_COLUMN") ==  0) {
                this->memory_controller_array[i]->set_address_mask_type(MEMORY_CONTROLLER_MASK_ROW_BANK_CHANNEL_COLUMN);
            }
            else if (strcasecmp(cfg_memory_controller[ memory_controller_parameters.back() ], "ROW_BANK_COLUMN") ==  0) {
                this->memory_controller_array[i]->set_address_mask_type(MEMORY_CONTROLLER_MASK_ROW_BANK_COLUMN);
            }
            else {
                ERROR_PRINTF("MAIN MEMORY %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_memory_controller[ memory_controller_parameters.back() ].c_str(), memory_controller_parameters.back());
            }

            memory_controller_parameters.push_back("LINE_SIZE");
            this->memory_controller_array[i]->set_line_size( cfg_memory_controller[ memory_controller_parameters.back() ] );
            this->set_global_line_size(this->memory_controller_array[i]->get_line_size());

            memory_controller_parameters.push_back("CONTROLLER_NUMBER");
            this->memory_controller_array[i]->set_controller_number( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("TOTAL_CONTROLLERS");
            this->memory_controller_array[i]->set_total_controllers( cfg_memory_controller[ memory_controller_parameters.back() ] );


            memory_controller_parameters.push_back("MSHR_BUFFER_REQUEST_RESERVED_SIZE");
            this->memory_controller_array[i]->set_mshr_buffer_request_reserved_size( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("MSHR_BUFFER_WRITEBACK_RESERVED_SIZE");
            this->memory_controller_array[i]->set_mshr_buffer_writeback_reserved_size( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("MSHR_BUFFER_PREFETCH_RESERVED_SIZE");
            this->memory_controller_array[i]->set_mshr_buffer_prefetch_reserved_size( cfg_memory_controller[ memory_controller_parameters.back() ] );


            memory_controller_parameters.push_back("CHANNELS_PER_CONTROLLER");
            this->memory_controller_array[i]->set_channels_per_controller( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("BANK_PER_CHANNEL");
            this->memory_controller_array[i]->set_bank_per_channel( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("BANK_BUFFER_SIZE");
            this->memory_controller_array[i]->set_bank_buffer_size( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("BANK_ROW_BUFFER_SIZE");
            this->memory_controller_array[i]->set_bank_row_buffer_size( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("BANK_SELECTION_POLICY");
            if (strcasecmp(cfg_memory_controller[ memory_controller_parameters.back() ], "RANDOM") ==  0) {
                this->memory_controller_array[i]->set_bank_selection_policy(SELECTION_RANDOM);
            }
            else if (strcasecmp(cfg_memory_controller[ memory_controller_parameters.back() ], "ROUND_ROBIN") ==  0) {
                this->memory_controller_array[i]->set_bank_selection_policy(SELECTION_ROUND_ROBIN);
            }
            else if (strcasecmp(cfg_memory_controller[ memory_controller_parameters.back() ], "BUFFER_LEVEL") ==  0) {
                this->memory_controller_array[i]->set_bank_selection_policy(SELECTION_BUFFER_LEVEL);
            }
            else {
                ERROR_PRINTF("MAIN MEMORY %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_memory_controller[ memory_controller_parameters.back() ].c_str(), memory_controller_parameters.back());
            }

            memory_controller_parameters.push_back("REQUEST_PRIORITY_POLICY");
            if (strcasecmp(cfg_memory_controller[ memory_controller_parameters.back() ], "ROW_BUFFER_HITS_FIRST") ==  0) {
                this->memory_controller_array[i]->set_request_priority_policy(REQUEST_PRIORITY_ROW_BUFFER_HITS_FIRST);
            }
            else if (strcasecmp(cfg_memory_controller[ memory_controller_parameters.back() ], "FIRST_COME_FIRST_SERVE") ==  0) {
                this->memory_controller_array[i]->set_request_priority_policy(REQUEST_PRIORITY_FIRST_COME_FIRST_SERVE);
            }
            else {
                ERROR_PRINTF("MAIN MEMORY %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_memory_controller[ memory_controller_parameters.back() ].c_str(), memory_controller_parameters.back());
            }

            memory_controller_parameters.push_back("WRITE_PRIORITY_POLICY");
            if (strcasecmp(cfg_memory_controller[ memory_controller_parameters.back() ], "SERVICE_AT_NO_READ") ==  0) {
                this->memory_controller_array[i]->set_write_priority_policy(WRITE_PRIORITY_SERVICE_AT_NO_READ);
            }
            else if (strcasecmp(cfg_memory_controller[ memory_controller_parameters.back() ], "DRAIN_WHEN_FULL") ==  0) {
                this->memory_controller_array[i]->set_write_priority_policy(WRITE_PRIORITY_DRAIN_WHEN_FULL);
            }
            else {
                ERROR_PRINTF("MAIN MEMORY %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_memory_controller[ memory_controller_parameters.back() ].c_str(), memory_controller_parameters.back());
            }

            /// DRAM configuration

            memory_controller_parameters.push_back("BUS_FREQUENCY");
            this->memory_controller_array[i]->set_bus_frequency( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("BURST_LENGTH");
            this->memory_controller_array[i]->set_burst_length( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("CORE_TO_BUS_CLOCK_RATIO");
            this->memory_controller_array[i]->set_core_to_bus_clock_ratio( cfg_memory_controller[ memory_controller_parameters.back() ] );

            /// DRAM timming configuration

            memory_controller_parameters.push_back("TIMING_AL");
            this->memory_controller_array[i]->set_timing_al( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("TIMING_CAS");
            this->memory_controller_array[i]->set_timing_cas( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("TIMING_CCD");
            this->memory_controller_array[i]->set_timing_ccd( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("TIMING_CWD");
            this->memory_controller_array[i]->set_timing_cwd( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("TIMING_FAW");
            this->memory_controller_array[i]->set_timing_faw( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("TIMING_RAS");
            this->memory_controller_array[i]->set_timing_ras( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("TIMING_RC");
            this->memory_controller_array[i]->set_timing_rc( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("TIMING_RCD");
            this->memory_controller_array[i]->set_timing_rcd( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("TIMING_RP");
            this->memory_controller_array[i]->set_timing_rp( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("TIMING_RRD");
            this->memory_controller_array[i]->set_timing_rrd( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("TIMING_RTP");
            this->memory_controller_array[i]->set_timing_rtp( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("TIMING_WR");
            this->memory_controller_array[i]->set_timing_wr( cfg_memory_controller[ memory_controller_parameters.back() ] );

            memory_controller_parameters.push_back("TIMING_WTR");
            this->memory_controller_array[i]->set_timing_wtr( cfg_memory_controller[ memory_controller_parameters.back() ] );


            this->memory_controller_array[i]->set_max_ports(1);

            /// ================================================================
            /// Check if any MEMORY_CONTROLLER non-required parameters exist
            for (int32_t j = 0 ; j < cfg_memory_controller.getLength(); j++) {
                bool is_required = false;
                for (uint32_t k = 0 ; k < memory_controller_parameters.size(); k++) {
                    if (strcmp(cfg_memory_controller[j].getName(), memory_controller_parameters[k]) == 0) {
                        is_required = true;
                        break;
                    }
                }
                ERROR_ASSERT_PRINTF(is_required, "MEMORY_CONTROLLER %d has PARAMETER not required: \"%s\"\n", i, cfg_memory_controller[j].getName());
            }

        }
        catch(libconfig::SettingNotFoundException &nfex) {
            ERROR_PRINTF(" MEMORY_CONTROLLER %d has required PARAMETER missing: \"%s\"\n", i, memory_controller_parameters.back());
        }
        catch(libconfig::SettingTypeException &tex) {
            ERROR_PRINTF(" MEMORY_CONTROLLER %d has PARAMETER wrong type: \"%s\"\n", i, memory_controller_parameters.back());
        }
    }
};

/// ============================================================================
void sinuca_engine_t::initialize_interconnection_router() {
    libconfig::Config cfg;
    cfg.setIncludeDir(this->arg_configuration_path);
    cfg.readFile(this->arg_configuration_file_name);

    libconfig::Setting &cfg_root = cfg.getRoot();
    libconfig::Setting &cfg_interconnection_router_list = cfg_root["INTERCONNECTION_ROUTER"];
    SINUCA_PRINTF("INTERCONNECTION_ROUTERS:%d\n", cfg_interconnection_router_list.getLength());

    this->set_interconnection_router_array_size(cfg_interconnection_router_list.getLength());
    this->interconnection_router_array = utils_t::template_allocate_initialize_array<interconnection_router_t*>(this->interconnection_router_array_size, NULL);

    /// ========================================================================
    /// Required CACHE MEMORY Parameters
    /// ========================================================================
    for (int32_t i = 0; i < cfg_interconnection_router_list.getLength(); i++) {
        this->interconnection_router_array[i] = new interconnection_router_t;

        container_ptr_const_char_t interconnection_router_parameters;
        libconfig::Setting &cfg_interconnection_router = cfg_interconnection_router_list[i];

        /// ====================================================================
        /// INTERCONNECTION_ROUTER PARAMETERS
        /// ====================================================================
        try {
            interconnection_router_parameters.push_back("LABEL");
            this->interconnection_router_array[i]->set_label( cfg_interconnection_router[ interconnection_router_parameters.back() ] );

            interconnection_router_parameters.push_back("INTERCONNECTION_LATENCY");
            this->interconnection_router_array[i]->set_interconnection_latency( cfg_interconnection_router[ interconnection_router_parameters.back() ] );

            interconnection_router_parameters.push_back("INTERCONNECTION_WIDTH");
            this->interconnection_router_array[i]->set_interconnection_width( cfg_interconnection_router[ interconnection_router_parameters.back() ] );

            interconnection_router_parameters.push_back("SELECTION_POLICY");
            if (strcasecmp(cfg_interconnection_router[ interconnection_router_parameters.back() ], "RANDOM") ==  0) {
                this->interconnection_router_array[i]->set_selection_policy(SELECTION_RANDOM);
            }
            else if (strcasecmp(cfg_interconnection_router[ interconnection_router_parameters.back() ], "ROUND_ROBIN") ==  0) {
                this->interconnection_router_array[i]->set_selection_policy(SELECTION_ROUND_ROBIN);
            }
            else if (strcasecmp(cfg_interconnection_router[ interconnection_router_parameters.back() ], "BUFFER_LEVEL") ==  0) {
                this->interconnection_router_array[i]->set_selection_policy(SELECTION_BUFFER_LEVEL);
            }
            else {
                ERROR_PRINTF("INTERCONNECTION_ROUTER %d found a strange VALUE %s for PARAMETER %s\n", i, cfg_interconnection_router[ interconnection_router_parameters.back() ].c_str(), interconnection_router_parameters.back());
            }

            interconnection_router_parameters.push_back("INPUT_BUFFER_SIZE");
            this->interconnection_router_array[i]->set_input_buffer_size( cfg_interconnection_router[ interconnection_router_parameters.back() ] );

            interconnection_router_parameters.push_back("CONNECTED_COMPONENT");
            this->interconnection_router_array[i]->set_max_ports( cfg_interconnection_router[ interconnection_router_parameters.back() ].getLength() );

            /// ================================================================
            /// Check if any INTERCONNECTION_ROUTER non-required parameters exist
            for (int32_t j = 0 ; j < cfg_interconnection_router.getLength(); j++) {
                bool is_required = false;
                for (uint32_t k = 0 ; k < interconnection_router_parameters.size(); k++) {
                    if (strcmp(cfg_interconnection_router[j].getName(), interconnection_router_parameters[k]) == 0) {
                        is_required = true;
                        break;
                    }
                }
                ERROR_ASSERT_PRINTF(is_required, "INTERCONNECTION_ROUTER %d has PARAMETER not required: \"%s\"\n", i, cfg_interconnection_router[j].getName());
            }

        }
        catch(libconfig::SettingNotFoundException &nfex) {
            ERROR_PRINTF(" INTERCONNECTION_ROUTER %d has required PARAMETER missing: \"%s\"\n", i, interconnection_router_parameters.back());
        }
        catch(libconfig::SettingTypeException &tex) {
            ERROR_PRINTF(" INTERCONNECTION_ROUTER %d has PARAMETER wrong type: \"%s\"\n", i, interconnection_router_parameters.back());
        }
    }
};

/// ============================================================================
void sinuca_engine_t::initialize_directory_controller() {
    libconfig::Config cfg;
    cfg.setIncludeDir(this->arg_configuration_path);
    cfg.readFile(this->arg_configuration_file_name);

    libconfig::Setting &cfg_root = cfg.getRoot();
    libconfig::Setting &cfg_directory = cfg_root["DIRECTORY_CONTROLLER"];
    SINUCA_PRINTF("DIRECTORY_CONTROLLER\n");

    /// ========================================================================
    /// Required DIRECTORY Parameters
    /// ========================================================================
    this->directory_controller = new directory_controller_t;
    container_ptr_const_char_t directory_parameters;
    /// ====================================================================
    /// DIRECTORY PARAMETERS
    /// ====================================================================
    try {
        directory_parameters.push_back("COHERENCE_PROTOCOL");
        if (strcasecmp(cfg_directory[ directory_parameters.back() ], "MOESI") ==  0) {
            this->directory_controller->set_coherence_protocol_type(COHERENCE_PROTOCOL_MOESI);
        }
        else {
            ERROR_PRINTF("DIRECTORY found a strange VALUE %s for PARAMETER %s\n", cfg_directory[ directory_parameters.back() ].c_str(), directory_parameters.back());
        }

        directory_parameters.push_back("INCLUSIVENESS");
        if (strcasecmp(cfg_directory[ directory_parameters.back() ], "NON_INCLUSIVE") ==  0) {
            this->directory_controller->set_inclusiveness_type(INCLUSIVENESS_NON_INCLUSIVE);
        }
        else if (strcasecmp(cfg_directory[ directory_parameters.back() ], "INCLUSIVE_ALL") ==  0) {
            this->directory_controller->set_inclusiveness_type(INCLUSIVENESS_INCLUSIVE_ALL);
        }
        else if (strcasecmp(cfg_directory[ directory_parameters.back() ], "INCLUSIVE_LLC") ==  0) {
            this->directory_controller->set_inclusiveness_type(INCLUSIVENESS_INCLUSIVE_LLC);
        }
        else {
            ERROR_PRINTF("DIRECTORY found a strange VALUE %s for PARAMETER %s\n", cfg_directory[ directory_parameters.back() ].c_str(), directory_parameters.back());
        }

        directory_parameters.push_back("GENERATE_LLC_WRITEBACK");
        this->directory_controller->set_generate_llc_writeback( cfg_directory[ directory_parameters.back() ] );

        directory_parameters.push_back("GENERATE_NON_LLC_WRITEBACK");
        this->directory_controller->set_generate_non_llc_writeback( cfg_directory[ directory_parameters.back() ] );

        directory_parameters.push_back("FINAL_WRITE_BACK_ALL");
        this->directory_controller->set_final_writeback_all( cfg_directory[ directory_parameters.back() ] );

        /// ================================================================
        /// Check if any DIRECTORY non-required parameters exist
        for (int32_t j = 0 ; j < cfg_directory.getLength(); j++) {
            bool is_required = false;
            for (uint32_t k = 0 ; k < directory_parameters.size(); k++) {
                if (strcmp(cfg_directory[j].getName(), directory_parameters[k]) == 0) {
                    is_required = true;
                    break;
                }
            }
            ERROR_ASSERT_PRINTF(is_required, "DIRECTORY has PARAMETER not required: \"%s\"\n", cfg_directory[j].getName());
        }

    }
    catch(libconfig::SettingNotFoundException &nfex) {
        ERROR_PRINTF(" DIRECTORY has required PARAMETER missing: \"%s\"\n", directory_parameters.back());
    }
    catch(libconfig::SettingTypeException &tex) {
        ERROR_PRINTF(" DIRECTORY has PARAMETER wrong type: \"%s\"\n", directory_parameters.back());
    }
};

/// ============================================================================
void sinuca_engine_t::initialize_interconnection_controller() {
    libconfig::Config cfg;
    cfg.setIncludeDir(this->arg_configuration_path);
    cfg.readFile(this->arg_configuration_file_name);

    libconfig::Setting &cfg_root = cfg.getRoot();
    libconfig::Setting &cfg_interconnection_controller = cfg_root["INTERCONNECTION_CONTROLLER"];
    SINUCA_PRINTF("INTERCONNECTION_CONTROLLER\n");

    /// ========================================================================
    /// Required INTERCONNECTION_CONTROLLER Parameters
    /// ========================================================================
    this->interconnection_controller = new interconnection_controller_t;
    container_ptr_const_char_t interconnection_controller_parameters;

    /// ====================================================================
    /// INTERCONNECTION_CONTROLLER PARAMETERS
    /// ====================================================================
    try {
        interconnection_controller_parameters.push_back("ROUTING_ALGORITHM");
        if (strcasecmp(cfg_interconnection_controller[ interconnection_controller_parameters.back() ], "XY") ==  0) {
            this->interconnection_controller->set_routing_algorithm(ROUTING_ALGORITHM_XY);
        }
        else if (strcasecmp(cfg_interconnection_controller[ interconnection_controller_parameters.back() ], "ODD_EVEN") ==  0) {
            this->interconnection_controller->set_routing_algorithm(ROUTING_ALGORITHM_ODD_EVEN);
        }
        else if (strcasecmp(cfg_interconnection_controller[ interconnection_controller_parameters.back() ], "FLOYD_WARSHALL") ==  0) {
            this->interconnection_controller->set_routing_algorithm(ROUTING_ALGORITHM_FLOYD_WARSHALL);
        }
        else {
            ERROR_PRINTF("INTERCONNECTION_CONTROLLER found a strange VALUE %s for PARAMETER %s\n", cfg_interconnection_controller[ interconnection_controller_parameters.back() ].c_str(), interconnection_controller_parameters.back());
        }

        /// ================================================================
        /// Check if any INTERCONNECTION_CONTROLLER non-required parameters exist
        for (int32_t j = 0 ; j < cfg_interconnection_controller.getLength(); j++) {
            bool is_required = false;
            for (uint32_t k = 0 ; k < interconnection_controller_parameters.size(); k++) {
                if (strcmp(cfg_interconnection_controller[j].getName(), interconnection_controller_parameters[k]) == 0) {
                    is_required = true;
                    break;
                }
            }
            ERROR_ASSERT_PRINTF(is_required, "INTERCONNECTION_CONTROLLER has PARAMETER not required: \"%s\"\n", cfg_interconnection_controller[j].getName());
        }

    }
    catch(libconfig::SettingNotFoundException &nfex) {
        ERROR_PRINTF(" INTERCONNECTION_CONTROLLER has required PARAMETER missing: \"%s\"\n", interconnection_controller_parameters.back());
    }
    catch(libconfig::SettingTypeException &tex) {
        ERROR_PRINTF(" INTERCONNECTION_CONTROLLER has PARAMETER wrong type: \"%s\"\n", interconnection_controller_parameters.back());
    }

};


/// ============================================================================
void sinuca_engine_t::make_connections() {
    libconfig::Config cfg;
    cfg.setIncludeDir(this->arg_configuration_path);
    cfg.readFile(this->arg_configuration_file_name);
    libconfig::Setting &cfg_root = cfg.getRoot();

    /// ========================================================================
    /// PROCESSOR => CACHE_MEMORY
    /// ========================================================================
    libconfig::Setting &cfg_processor_list = cfg_root["PROCESSOR"];
    for (int32_t i = 0; i < cfg_processor_list.getLength(); i++) {
        bool found_component;
        libconfig::Setting &cfg_processor = cfg_processor_list[i];

        /// ====================================================================
        /// Connected DATA CACHE_MEMORY
        /// ====================================================================
        found_component = false;
        const char *data_cache_memory_label = cfg_processor[ "CONNECTED_DATA_CACHE" ];
        for (uint32_t component = 0; component < this->get_cache_memory_array_size(); component++) {
            if (strcmp(this->cache_memory_array[component]->get_label(), data_cache_memory_label) == 0) {
                /// Connect the Data Cache to the Core
                this->processor_array[i]->set_data_cache(this->cache_memory_array[component]);
                interconnection_interface_t *obj_A = this->processor_array[i];
                uint32_t port_A = PROCESSOR_PORT_DATA_CACHE;

                interconnection_interface_t *obj_B = this->cache_memory_array[component];
                uint32_t port_B = this->cache_memory_array[component]->get_used_ports();

                CONFIGURATOR_DEBUG_PRINTF("Linking %s(%u) to %s(%u)\n\n", obj_A->get_label(), port_A, obj_B->get_label(), port_B);
                this->processor_array[i]->set_higher_lower_level_component( obj_A, port_A, obj_B, port_B);
                this->processor_array[i]->add_used_ports();
                this->cache_memory_array[component]->add_used_ports();
                found_component = true;
                break;
            }
        }
        ERROR_ASSERT_PRINTF(found_component == true, "PROCESSOR has a not found CONNECTED_DATA_CACHE:\"%s\"\n", data_cache_memory_label);

        /// ====================================================================
        /// Connected INST CACHE_MEMORY
        /// ====================================================================
        found_component = false;
        const char *inst_cache_memory_label = cfg_processor[ "CONNECTED_INST_CACHE" ];
        for (uint32_t component = 0; component < this->get_cache_memory_array_size(); component++) {
            if (strcmp(this->cache_memory_array[component]->get_label(), inst_cache_memory_label) == 0) {
                /// Connect the Inst Cache to the Core
                this->processor_array[i]->set_inst_cache(this->cache_memory_array[component]);
                interconnection_interface_t *obj_A = this->processor_array[i];
                uint32_t port_A = PROCESSOR_PORT_INST_CACHE;

                interconnection_interface_t *obj_B = this->cache_memory_array[component];
                uint32_t port_B = this->cache_memory_array[component]->get_used_ports();

                CONFIGURATOR_DEBUG_PRINTF("Linking %s(%u) to %s(%u)\n\n", obj_A->get_label(), port_A, obj_B->get_label(), port_B);
                this->processor_array[i]->set_higher_lower_level_component( obj_A, port_A, obj_B, port_B);
                this->processor_array[i]->add_used_ports();
                this->cache_memory_array[component]->add_used_ports();
                found_component = true;
                break;
            }
        }
        ERROR_ASSERT_PRINTF(found_component == true, "PROCESSOR has a not found CONNECTED_INST_CACHE:\"%s\"\n", inst_cache_memory_label);
    }

    /// ========================================================================
    /// CACHE_MEMORY => LOWER_LEVEL_CACHE
    /// ========================================================================
    libconfig::Setting &cfg_cache_memory_list = cfg_root["CACHE_MEMORY"];
    for (int32_t i = 0; i < cfg_cache_memory_list.getLength(); i++) {
        /// ====================================================================
        /// LOWER_LEVEL_CACHE items
        /// ====================================================================
        libconfig::Setting &cfg_lower_level_cache = cfg_cache_memory_list[i][ "LOWER_LEVEL_CACHE" ];
        for (int32_t j = 0; j < cfg_lower_level_cache.getLength(); j++) {
            bool found_component = false;
            const char *cache_label = cfg_lower_level_cache[j];
            for (uint32_t component = 0; component < this->get_cache_memory_array_size(); component++) {
                if (strcmp(this->cache_memory_array[component]->get_label(), cache_label) == 0) {
                    /// Connect the Lower Level
                    this->cache_memory_array[i]->add_lower_level_cache(this->cache_memory_array[component]);
                    /// Connect the Higher Level
                    this->cache_memory_array[component]->add_higher_level_cache(this->cache_memory_array[i]);
                    found_component = true;
                    break;
                }
            }
            ERROR_ASSERT_PRINTF(found_component == true, "CACHE_MEMORY has a not found LOWER_LEVEL_CACHE:\"%s\"\n", cache_label);
        }
    }

    /// ========================================================================
    /// INTERCONNECTION_ROUTER => CONNECTED_COMPONENTS
    /// ========================================================================
    libconfig::Setting &cfg_interconnection_router_list = cfg_root["INTERCONNECTION_ROUTER"];
    for (int32_t i = 0; i < cfg_interconnection_router_list.getLength(); i++) {
        /// ====================================================================
        /// Connected INTERCONNECTION_INTERFACE
        /// ====================================================================
        libconfig::Setting &cfg_connected_component = cfg_interconnection_router_list[i][ "CONNECTED_COMPONENT" ];
        CONFIGURATOR_DEBUG_PRINTF("Component %s connections\n", this->interconnection_router_array[i]->get_label());
        for (int32_t j = 0; j < cfg_connected_component.getLength(); j++) {
            bool found_component = false;
            const char *connected_component_label = cfg_connected_component[j];
            for (uint32_t component = 0; component < this->get_interconnection_interface_array_size(); component++) {
                if (strcmp(this->interconnection_interface_array[component]->get_label(), connected_component_label) == 0) {
                    interconnection_interface_t *obj_A = this->interconnection_router_array[i];
                    interconnection_interface_t *obj_B = this->interconnection_interface_array[component];
                    uint32_t port_A = 0;
                    uint32_t port_B = 0;

                    int32_t port_A_exist = obj_A->find_port_to_obj(obj_B);
                    /// If is a connect between routers
                    if (port_A_exist != POSITION_FAIL) {
                        port_A = port_A_exist;
                    }
                    else {
                        port_A = obj_A->get_used_ports();
                        obj_A->add_used_ports();
                    }

                    int32_t port_B_exist = obj_B->find_port_to_obj(obj_A);
                    /// Check if already connected
                    if (port_B_exist != POSITION_FAIL) {
                        port_B = port_B_exist;
                    }
                    else {
                        port_B = obj_B->get_used_ports();
                        obj_B->add_used_ports();
                    }

                    CONFIGURATOR_DEBUG_PRINTF("Linking %s(%u) to %s(%u)\n", obj_A->get_label(), port_A, obj_B->get_label(), port_B);
                    obj_A->set_higher_lower_level_component(obj_A, port_A, obj_B, port_B);
                    found_component = true;
                    break;
                }
            }
            ERROR_ASSERT_PRINTF(found_component == true, "ROUTER has a not found CONNECTED_COMPONENT:\"%s\"\n", connected_component_label);
        }
        CONFIGURATOR_DEBUG_PRINTF("========================\n");
    }
};
