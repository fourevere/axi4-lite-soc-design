
`timescale 1 ns / 1 ps

	module uart_v1_0_S00_AXI #
	(
		parameter integer C_S_AXI_DATA_WIDTH	= 32,
		parameter integer C_S_AXI_ADDR_WIDTH	= 4
	)
	(
		//User to add ports here
		output wire [7:0] 	tx_data,
		output wire 		tx_valid,
		output wire 		tx_ready,

		input wire [7:0] 	rx_data,
		input wire 			rx_valid,
		output wire 		rx_ie		,

		//User ports ends
		//Do not modify the ports beyond this line

		//global signals
		input wire  S_AXI_ACLK,
		input wire  S_AXI_ARESETN,
		//AW channel
		input wire [C_S_AXI_ADDR_WIDTH-1 : 0] S_AXI_AWADDR,
		input wire [2 : 0] S_AXI_AWPROT,
		input wire  S_AXI_AWVALID,
		output wire  S_AXI_AWREADY,
		//W channel
		input wire [C_S_AXI_DATA_WIDTH-1 : 0] S_AXI_WDATA,  
		input wire [(C_S_AXI_DATA_WIDTH/8)-1 : 0] S_AXI_WSTRB,
		input wire  S_AXI_WVALID,
		output wire  S_AXI_WREADY,
		//B channel
		output wire [1 : 0] S_AXI_BRESP,
		output wire  S_AXI_BVALID,
		input wire  S_AXI_BREADY,
		//AR channel
		input wire [C_S_AXI_ADDR_WIDTH-1 : 0] S_AXI_ARADDR,
		input wire [2 : 0] S_AXI_ARPROT,
		input wire  S_AXI_ARVALID,
		output wire  S_AXI_ARREADY,
		//R channel
		output wire [C_S_AXI_DATA_WIDTH-1 : 0] S_AXI_RDATA,
		output wire [1 : 0] S_AXI_RRESP,
		output wire  S_AXI_RVALID,
		input wire  S_AXI_RREADY
	);

	// AXI4LITE signals
	reg [C_S_AXI_ADDR_WIDTH-1 : 0] 	axi_awaddr;
	reg  	axi_awready;
	reg  	axi_wready;
	reg [1 : 0] 	axi_bresp;
	reg  	axi_bvalid;
	reg [C_S_AXI_ADDR_WIDTH-1 : 0] 	axi_araddr;
	reg  	axi_arready;
	reg [C_S_AXI_DATA_WIDTH-1 : 0] 	axi_rdata;
	reg [1 : 0] 	axi_rresp;
	reg  	axi_rvalid;

	// Example-specific design signals
	// local parameter for addressing 32 bit / 64 bit C_S_AXI_DATA_WIDTH
	// ADDR_LSB is used for addressing 32/64 bit registers/memories
	// ADDR_LSB = 2 for 32 bits (n downto 2)
	// ADDR_LSB = 3 for 64 bits (n downto 3)
	localparam integer ADDR_LSB = (C_S_AXI_DATA_WIDTH/32) + 1;
	localparam integer OPT_MEM_ADDR_BITS = 1;
	//----------------------------------------------
	//-- Signals for user logic register space example
	//------------------------------------------------
	//-- Number of Slave Registers 4
	reg [C_S_AXI_DATA_WIDTH-1:0]	uart_sr ; //slv_reg0
	reg [C_S_AXI_DATA_WIDTH-1:0]	uart_tdr; //slv_reg1
	reg [C_S_AXI_DATA_WIDTH-1:0]	uart_rdr; //slv_reg2
	reg [C_S_AXI_DATA_WIDTH-1:0]	uart_cr;  //slv_reg3
	
	wire	 slv_reg_rden;
	wire	 slv_reg_wren;
	reg [C_S_AXI_DATA_WIDTH-1:0]	 reg_data_out;
	integer	 byte_index;
	reg	 aw_en;


	reg tx_valid_r;
	reg rx_flag;

	// I/O Connections assignments
	assign tx_data 	= uart_tdr[7:0];
	assign tx_valid = tx_valid_r;
	assign rx_ie 	= uart_cr[0];

	assign S_AXI_AWREADY	= axi_awready;
	assign S_AXI_WREADY	= axi_wready;
	assign S_AXI_BRESP	= axi_bresp;
	assign S_AXI_BVALID	= axi_bvalid;
	assign S_AXI_ARREADY	= axi_arready;
	assign S_AXI_RDATA	= axi_rdata;
	assign S_AXI_RRESP	= axi_rresp;
	assign S_AXI_RVALID	= axi_rvalid;
	// Implement axi_awready generation
	// axi_awready is asserted for one S_AXI_ACLK clock cycle when both
	// S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_awready is
	// de-asserted when reset is low.

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_awready <= 1'b0;
	      aw_en <= 1'b1;
	    end 
	  else
	    begin    
	      if (~axi_awready && S_AXI_AWVALID && S_AXI_WVALID && aw_en)
	        begin
	          // slave is ready to accept write address when 
	          // there is a valid write address and write data
	          // on the write address and data bus. This design 
	          // expects no outstanding transactions. 
	          axi_awready <= 1'b1;
	          aw_en <= 1'b0;
	        end
	        else if (S_AXI_BREADY && axi_bvalid)
	            begin
	              aw_en <= 1'b1;
	              axi_awready <= 1'b0;
	            end
	      else           
	        begin
	          axi_awready <= 1'b0;
	        end
	    end 
	end       

	// Implement axi_awaddr latching
	// This process is used to latch the address when both 
	// S_AXI_AWVALID and S_AXI_WVALID are valid. 

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_awaddr <= 0;
	    end 
	  else
	    begin    
	      if (~axi_awready && S_AXI_AWVALID && S_AXI_WVALID && aw_en)
	        begin
	          // Write Address latching 
	          axi_awaddr <= S_AXI_AWADDR;
	        end
	    end 
	end       

	// Implement axi_wready generation
	// axi_wready is asserted for one S_AXI_ACLK clock cycle when both
	// S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_wready is 
	// de-asserted when reset is low. 

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_wready <= 1'b0;
	    end 
	  else
	    begin    
	      if (~axi_wready && S_AXI_WVALID && S_AXI_AWVALID && aw_en )
	        begin
	          // slave is ready to accept write data when 
	          // there is a valid write address and write data
	          // on the write address and data bus. This design 
	          // expects no outstanding transactions. 
	          axi_wready <= 1'b1;
	        end
	      else
	        begin
	          axi_wready <= 1'b0;
	        end
	    end 
	end       

	// Implement memory mapped register select and write logic generation
	// The write data is accepted and written to memory mapped registers when
	// axi_awready, S_AXI_WVALID, axi_wready and S_AXI_WVALID are asserted. Write strobes are used to
	// select byte enables of slave registers while writing.
	// These registers are cleared when reset (active low) is applied.
	// Slave register write enable is asserted when valid address and data are available
	// and the slave is ready to accept the write address and write data.



	assign slv_reg_wren = axi_wready && S_AXI_WVALID && axi_awready && S_AXI_AWVALID;
	// SR은 상태 표시라 cpu가 write 할 필요가 없다. cpu가 read만 한다.
	// always @( posedge S_AXI_ACLK ) begin
	// 	if ( S_AXI_ARESETN == 1'b0 ) begin
	// 		uart_sr   	<= 0;
	// 	end 
	// 	else begin
	// 		if (slv_reg_wren && axi_awaddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] == 2'h3) begin
	// 		    uart_sr <= S_AXI_WDATA;
	// 		end
	// 	end
	// end 


	always @( posedge S_AXI_ACLK ) begin
		if ( S_AXI_ARESETN == 1'b0 ) begin
			uart_tdr   	<= 0;
			tx_valid_r 	  <= 1'b0;
		end 
		else begin
			tx_valid_r 	  <= 1'b0;
			if (slv_reg_wren && axi_awaddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] == 2'h1) begin
		    	if ( S_AXI_WSTRB[0] == 1 ) begin
		    		uart_tdr[7:0] <= S_AXI_WDATA[7:0];
					tx_valid_r 	  <= 1'b1;
		    	end
		    end
		end
	end


	always @( posedge S_AXI_ACLK ) begin
		if ( S_AXI_ARESETN == 1'b0 ) begin
			uart_rdr   	<= 0;
			rx_flag 	<= 1'b0;
		end 
		else begin
			if(rx_valid) begin
				uart_rdr 	<= rx_data; // Latching
				rx_flag 	<= 1'b1;
			end
			if (slv_reg_rden && axi_araddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] == 2'h2) begin
				rx_flag <= 1'b0;
			end
		end
	end


	always @( posedge S_AXI_ACLK ) begin
		if ( S_AXI_ARESETN == 1'b0 ) begin
			uart_cr   	<= 0;
		end 
		else begin
			if (slv_reg_wren && axi_awaddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] == 2'h3) begin
			    uart_cr <= S_AXI_WDATA;
			end
		end
	end 

	// Implement write response logic generation
	// The write response and response valid signals are asserted by the slave 
	// when axi_wready, S_AXI_WVALID, axi_wready and S_AXI_WVALID are asserted.  
	// This marks the acceptance of address and indicates the status of 
	// write transaction.

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_bvalid  <= 0;
	      axi_bresp   <= 2'b0;
	    end 
	  else
	    begin    
	      if (axi_awready && S_AXI_AWVALID && ~axi_bvalid && axi_wready && S_AXI_WVALID)
	        begin
	          // indicates a valid write response is available
	          axi_bvalid <= 1'b1;
	          axi_bresp  <= 2'b0; // 'OKAY' response 
	        end                   // work error responses in future
	      else
	        begin
	          if (S_AXI_BREADY && axi_bvalid) 
	            //check if bready is asserted while bvalid is high) 
	            //(there is a possibility that bready is always asserted high)   
	            begin
	              axi_bvalid <= 1'b0; 
	            end  
	        end
	    end
	end   

	// Implement axi_arready generation
	// axi_arready is asserted for one S_AXI_ACLK clock cycle when
	// S_AXI_ARVALID is asserted. axi_awready is 
	// de-asserted when reset (active low) is asserted. 
	// The read address is also latched when S_AXI_ARVALID is 
	// asserted. axi_araddr is reset to zero on reset assertion.

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_arready <= 1'b0;
	      axi_araddr  <= 32'b0;
	    end 
	  else
	    begin    
	      if (~axi_arready && S_AXI_ARVALID)
	        begin
	          // indicates that the slave has acceped the valid read address
	          axi_arready <= 1'b1;
	          // Read address latching
	          axi_araddr  <= S_AXI_ARADDR;
	        end
	      else
	        begin
	          axi_arready <= 1'b0;
	        end
	    end 
	end       

	// Implement axi_arvalid generation
	// axi_rvalid is asserted for one S_AXI_ACLK clock cycle when both 
	// S_AXI_ARVALID and axi_arready are asserted. The slave registers 
	// data are available on the axi_rdata bus at this instance. The 
	// assertion of axi_rvalid marks the validity of read data on the 
	// bus and axi_rresp indicates the status of read transaction.axi_rvalid 
	// is deasserted on reset (active low). axi_rresp and axi_rdata are 
	// cleared to zero on reset (active low).  
	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_rvalid <= 0;
	      axi_rresp  <= 0;
	    end 
	  else
	    begin    
	      if (axi_arready && S_AXI_ARVALID && ~axi_rvalid)
	        begin
	          // Valid read data is available at the read data bus
	          axi_rvalid <= 1'b1;
	          axi_rresp  <= 2'b0; // 'OKAY' response
	        end   
	      else if (axi_rvalid && S_AXI_RREADY)
	        begin
	          // Read data is accepted by the master
	          axi_rvalid <= 1'b0;
	        end                
	    end
	end    

	// Implement memory mapped register select and read logic generation
	// Slave register read enable is asserted when valid address is available
	// and the slave is ready to accept the read address.
	assign slv_reg_rden = axi_arready & S_AXI_ARVALID & ~axi_rvalid;
	always @(*)
	begin
	      // Address decoding for reading registers
	      case ( axi_araddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] )
	        2'h0   : reg_data_out <= {{30{1'b0}},rx_flag,tx_ready};	//uart status register slv_reg0
	        2'h1   : reg_data_out <= uart_tdr;					//slv_reg1
	        2'h2   : reg_data_out <= uart_rdr;					//slv_reg2
	        2'h3   : reg_data_out <= uart_cr; 					//slv_reg3
	        default : reg_data_out <= 0;
	      endcase
	end

	// Output register or memory read data
	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_rdata  <= 0;
	    end 
	  else
	    begin    
	      // When there is a valid read address (S_AXI_ARVALID) with 
	      // acceptance of read address by the slave (axi_arready), 
	      // output the read dada 
	      if (slv_reg_rden)
	        begin
	          axi_rdata <= reg_data_out;     // register read data
	        end   
	    end
	end    

	// Add user logic here

	// User logic ends

	endmodule


	// ******************************************************************************************************************//
    	// output  wire          cnt_en      ,
    	// output  wire          intr_en     ,
    	// output  wire [31:0]   psc         ,
    	// output  wire [31:0]   arr         ,
    	// output  wire          cnt_valid   ,
    	// output  wire [31:0]   i_cnt       ,
    	// input wire [31:0]    o_cnt       ,
    	// input wire           intr   uart_sr[0]

			// always @( posedge S_AXI_ACLK )
	// begin
	//   if ( S_AXI_ARESETN == 1'b0 )
	//     begin
	//       uart_sr <= 0;
	//       uart_tdr <= 0;
	//       uart_rdr <= 0;
	//       uart_cr <= 0;
	//     end 
	//   else begin
	//     if (slv_reg_wren)
	//       begin
	//         case ( axi_awaddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] )
	//           2'h0:
	//             for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	//               if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	//                 // Respective byte enables are asserted as per write strobes 
	//                 // Slave register 0
	//                 uart_sr[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	//               end  
	//           2'h1:
	//             for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	//               if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	//                 // Respective byte enables are asserted as per write strobes 
	//                 // Slave register 1
	//                 uart_tdr[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	//               end  
	//           2'h2:
	//             for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	//               if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	//                 // Respective byte enables are asserted as per write strobes 
	//                 // Slave register 2
	//                 uart_rdr[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	//               end  
	//           2'h3:
	//             for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	//               if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	//                 // Respective byte enables are asserted as per write strobes 
	//                 // Slave register 3
	//                 uart_cr[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	//               end  
	//           default : begin
	//                       uart_sr <= uart_sr;
	//                       uart_tdr <= uart_tdr;
	//                       uart_rdr <= uart_rdr;
	//                       uart_cr <= uart_cr;
	//                     end
	//         endcase
	//       end
	//   end
	// end    