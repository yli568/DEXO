// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

contract OptiSwap {
    address public seller;
    address public buyer;
    uint256 public price;
    string public auxiliaryInfo;
    bytes32 public r_z;
    bytes32 public commitment;
    bytes public key;
    bool public keyRevealed;

    event SellerInitialized(address seller, uint256 price, string auxInfo, bytes32 r_z, bytes32 commitment);
    event BuyerAccepted(address buyer, uint256 amount);
    event KeyRevealed(bytes key);
    event ChallengeInitiated(address indexed challenger, uint challengeCount, uint[] challenges);
    event ResponseSubmitted(address indexed responder, uint challengeCount, uint[] indices, bytes32[] values);
    enum stage {initialized, accepted, revealed, challenged, responded, finalized}



    uint public challengeLimit = 256;
    uint constant feeBuyer = 256;
    uint constant feeSeller = 256;
    uint constant proofLength = 256;
    uint[] public recentChallenges;
    struct Response {
        uint index;
        bytes32 value;
        bytes32[proofLength] proof;
    }
    Response[] public recentResponses;
    uint public timeout;
    stage public phase = stage.initialized;
    function nextStage(stage s) internal {
        phase = s;
        timeout = block.timestamp + 10 minutes;
    }

    constructor() {
        seller = msg.sender;
    }

    // Function for seller to initialize the contract (Round 1)
    function initialize(uint256 _price, string calldata _auxiliaryInfo, bytes32 _r_z, bytes32 _commitment) external {
        require(msg.sender == seller, "Only seller can initialize.");
        price = _price;
        auxiliaryInfo = _auxiliaryInfo;
        r_z = _r_z;
        commitment = _commitment;

        emit SellerInitialized(seller, price, auxiliaryInfo, r_z, commitment);
        nextStage(stage.initialized);
    }

    // Function for buyer to accept and transfer payment (Round 2)
    function accept() external payable {
        require(msg.value == price, "Incorrect payment amount.");
        require(phase == stage.initialized);
        buyer = msg.sender;
        emit BuyerAccepted(buyer, msg.value);
        nextStage(stage.accepted);
    }

    // Function for seller to reveal the key (Round 3)
    function revealKey(bytes calldata _key) external {
        require(msg.sender == seller, "Only seller can reveal the key.");
        require(!keyRevealed, "Key already revealed.");
        require(keccak256(abi.encodePacked(_key)) == commitment, "Invalid key.");
        require(phase == stage.accepted);
        key = _key;
        keyRevealed = true;
        emit KeyRevealed(key);
        nextStage(stage.revealed);
    }

    function challenge(uint[] calldata _Q) payable external {
        require(msg.sender == buyer);
        require(_Q.length <= challengeLimit);
        require(msg.value >= _Q.length * feeBuyer);
        require(phase == stage.revealed);
        recentChallenges = _Q;
        challengeLimit = challengeLimit - _Q.length;
        emit ChallengeInitiated(msg.sender, recentChallenges.length, recentChallenges);
        nextStage(stage.challenged);
    }
    
    function respond(uint[] calldata indices, bytes32[] calldata values, bytes32[proofLength][] calldata proofs) payable external {
        require(msg.sender == seller);
        require(indices.length == recentChallenges.length);
        require(values.length == recentChallenges.length);
        require(proofs.length == recentChallenges.length);
        require(msg.value >= recentChallenges.length * feeSeller);
        require(phase == stage.challenged);
        delete recentResponses;
        for (uint i = 0; i < recentChallenges.length; i++) {
            Response memory r = Response(indices[i], values[i], proofs[i]);
            recentResponses.push(r);
        }
        emit ResponseSubmitted(msg.sender, recentChallenges.length, indices, values);
        nextStage(stage.responded);
    }

    function refund() external {
        require(block.timestamp > timeout);
        if (phase == stage.revealed) payable(seller).transfer(address(this).balance);
        if (phase == stage.responded) payable(seller).transfer(address(this).balance);
        if (phase == stage.challenged) payable(buyer).transfer(address(this).balance);
    }

    function noComplain() external  {
        require(msg.sender == buyer);
        require(phase == stage.revealed);
        payable(seller).transfer(address(this).balance);
    }
}
