# DEXO System

The DEXO system involves five distinct components interacting together: Ethereum (or another Trusted Third Party), DEXO Nodes, Attestation Server, P-DApp Server, and buyer.

## Directory Structure

The project is organized into five corresponding folders, each representing one of the components mentioned above.

## Running an Experiment

To successfully run an experiment within the DEXO system, follow these steps:

1. **Start the DEXO Nodes and Attestation Server**: Ensure that these services are up and running before proceeding. Also, ensure that an Ethereum Node is available and operational.

2. **Compile the Smart Contract**:
   - Locate the DEXO Fair Exchange Contract within its directory.
   - Compile the smart contract to generate usable bytecode.

3. **Deploy the Smart Contract**:
   - Use the P-DApp Server to deploy the compiled smart contract on Ethereum with suitable parameters.

4. **Data Handling**:
   - The P-DApp Server will transmit received shares (stored in the data folder) to the DEXO Nodes.

5. **Verification and Transaction**:
   - DEXO Nodes will verify the shares' reliability through the Attestation Server.
   - Utilize fair exchange techniques to transmit metadata to the smart contract.
   - Upon declaration from the buyer wishing to make a purchase, use fair exchange techniques to trade shares with the buyer.

6. **Final Steps**:
   - Once the smart contract has been successfully deployed, you can run the buyer to await the data transaction.

## OP-TEE Secret Sharing Test

This folder contains code used to test the time it takes to run [Shamir's Secret Sharing](https://github.com/fletcher/c-sss) under the OP-TEE environment on an rpi3. For detailed instructions, please refer to [this guide](https://kickstartembedded.com/2022/11/07/op-tee-part-3-setting-up-op-tee-on-qemu-raspberry-pi-3/#google_vignette).


### How to Run the Test

1. **Prepare the Folder**:
   - Place the `OP-TEE_Secret_Sharing_Test` folder into the `examples` directory of the OP-TEE project.

2. **Compile the OP-TEE Project**:
   - Compile the entire OP-TEE project to generate a system image.

3. **Deploy on Raspberry Pi 3**:
   - Transfer the system image to an rpi3 and run the system.
