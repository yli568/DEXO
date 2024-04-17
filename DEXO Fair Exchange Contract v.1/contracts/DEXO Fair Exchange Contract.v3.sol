// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

contract FairExchange {
    address[] public dataSources;
    address[] public sellerNodes;
    address[] public badNodes;
    address public seller;
    address public buyer;
    uint256 public price;
    string public auxiliaryInfo;
    bytes32[] public r_z;
    bytes32[] public commitment;
    bytes[] public key;
    string buyerIp;
    uint256 buyerPort;
    uint256 revealedKeyNumber;
    stage[] public phase;

    event SellerInitialized(address seller, uint256 price, string auxiliaryInfo, address[] dataSources, address[] sellerNodes);
    event SellerNodeInitialized(address sellerNode, bytes32 r_z, bytes32 commitment);
    event BuyerClaimed(address buyer, string buyerIp, uint256 buyerPort);
    event BuyerAccepted(address sellerNode, uint256 amount);
    event KeyRevealed(address sllerNode, bytes key);
    enum stage {initialized, accepted, revealed}

    uint[] public timeout;
    // stage[] public phase = stage.initialized;
    function nextStage(uint i, stage s) internal {
        phase[i] = s;
        timeout[i] = block.timestamp + 10 minutes;
    }

    constructor(uint256 _price, string memory _auxiliaryInfo, address[] memory _dataSources, address[] memory _sellerNodes) {
        price = _price;
        auxiliaryInfo = _auxiliaryInfo;
        dataSources = _dataSources;
        seller = msg.sender;
        sellerNodes = _sellerNodes;
        revealedKeyNumber = 0;
        emit SellerInitialized(seller, price, auxiliaryInfo, dataSources, sellerNodes);
    }

    // Function for sellerNodes to initialize the contract
    function initialize(bytes32 _r_z, bytes32 _commitment) external {
        bool isSellerNode = false;
        uint i = 0;
        for (; i < sellerNodes.length; i++) {
            if (msg.sender == sellerNodes[i]) {
                isSellerNode = true;
                break;
            }
        }
        require(isSellerNode, "Only nodes in [sellerNodes] can initialize.");

        r_z[i] = _r_z;
        commitment[i] = _commitment;

        emit SellerNodeInitialized(sellerNodes[i], r_z[i], commitment[i]);
        nextStage(i, stage.initialized);
    }

    function buyerClaim(string calldata _ip, uint256 _port) external {
        buyer = msg.sender;
        buyerIp=_ip;
        buyerPort=_port;
        emit BuyerClaimed(buyer, buyerIp, buyerPort);
    }
    // After buyer reveals its ip and port, nodes send "z[]"s to buyer.

    // Function for buyer to accept and transfer payment. After buyer gets z[], buyer pays for key revealing.
    function accept(address sellerNode) external payable {
        require(msg.sender == buyer);
        bool isSellerNode = false;
        uint i = 0;
        for (; i < sellerNodes.length; i++) {
            if (sellerNode == sellerNodes[i]) {
                isSellerNode = true;
                break;
            }
        }
        require(isSellerNode, "sellerNode is not in [sellerNodes].");
        require(phase[i] == stage.initialized);
        require(msg.value == price, "Incorrect payment amount.");
        emit BuyerAccepted(sellerNodes[i], msg.value);
        nextStage(i, stage.accepted);
    }

    // Function for sellerNodes to reveal the key
    function revealKey(bytes calldata _key) external {
        bool isSellerNode = false;
        uint i = 0;
        for (; i < sellerNodes.length; i++) {
            if (msg.sender == sellerNodes[i]) {
                isSellerNode = true;
                break;
            }
        }
        require(isSellerNode, "Only nodes in [sellerNodes] can revealKey.");
        require(phase[i] == stage.accepted);
        require(keccak256(abi.encodePacked(_key)) == commitment[i], "Invalid key.");
        key[i] = _key;
        revealedKeyNumber += 1;
        emit KeyRevealed(sellerNodes[i], key[i]);
        nextStage(i, stage.revealed);
    }

    // Everyone can call this function.
    function noComplain() external  {
        require(revealedKeyNumber > sellerNodes.length/2 + 1);
        for(uint i=0; i < sellerNodes.length; i++){
            require(block.timestamp > timeout[i]);
        }
        uint256 sellerPayment = address(this).balance / 10;
        uint256 paymentPerDataSource = (address(this).balance - sellerPayment) / dataSources.length;
        for (uint j = 0; j < dataSources.length; j++) {
            payable(dataSources[j]).transfer(paymentPerDataSource);
        }
        selfdestruct(payable(seller));
    }

    function isValidChar(uint8 charCode) private pure returns (bool) {
        // '0'-'9' => 48-57, 'A'-'Z' => 65-90, 'a'-'z' => 97-122
        if((charCode >= 48 && charCode <= 57) || 
           (charCode >= 65 && charCode <= 90) || 
           (charCode >= 97 && charCode <= 122)) {
            return true;
        }
        return false;
    }

    // Challenge a bad node. A simple challenge.
    function challenge1(address badNode, bytes memory badShare, bytes32[] memory proofs, uint256 index) external {
        require(msg.sender == buyer);
        bool isSellerNode = false;
        uint256 i = 0;
        for (; i < sellerNodes.length; i++) {
            if (badNode == sellerNodes[i]) {
                isSellerNode = true;
                break;
            }
        }
        require(isSellerNode, "Only nodes in [sellerNodes] can be challenged.");
        bytes32 computedHash = keccak256(abi.encodePacked(badShare));
        for (i = 0; i < proofs.length; i++) {
            bytes32 proof = proofs[i];
            if (index % 2 == 0) {
                computedHash = keccak256(abi.encodePacked(computedHash, proof));
            } else {
                computedHash = keccak256(abi.encodePacked(proof, computedHash));
            }
            index = index / 2;
        }
        require(computedHash == r_z[i], "The share is not provided by sellerNodes.");
        bool isValid = true;
        for(i = 0; i < badShare.length; i++){
            if(!isValidChar(uint8(badShare[i]))){
                isValid = false;
            }
        }
        require(isValid == false, "The share is not garbled data. It's valid");
        
        // Return money to buyer
        payable(buyer).transfer(price);
        badNodes.push(badNode);
        revealedKeyNumber -= 1;
    }
}
