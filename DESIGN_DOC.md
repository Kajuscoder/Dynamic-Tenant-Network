# Explanation and Justification

## API Design Rationale
To fulfil the business needs & constraints specified in the problem statement, this RESTful web-server consists of three independent resources. 
* Networks: Manages all Tenant Networks in the systems. Each tenant network has a tenant_id. Each network may contain one or more than one  VXLAN network associated to it to support VNI stacking. VNIs in a tenant network can have different or same vtep_ip. In case of same vtep_ip, isolation can be achieved using VLAN. Request Body must have a tenant_id (string) and associated vtep_ip to create a new VXLAN network. Unique VNIs are allocated dynamically and returned in the response body. Information related to a specific VNI can be retrieved using query parameter (?vni=1).
* Endpoints: Manages all the endpoints (VMs) in the system. Each endpoint must have IP, MAC and Local Interface and this information needs to be sent through request-body. Response body will contain a unique endpoint_id that can uniquely identify an endpoint.
* Connections: Connects an endpoint to a specific VNI by associating a VM's interface/MAC with the VNI and a remote VTEP IP. Request body must have a VNI and an endpoint_id specified. Response body will return a unique connection_id. When a new connection is added or deleted, relevant linux commands are dumped in log file namely “commands.log”. Query parameters can be used to get details of connections with respect to a specific VNI (?vni=1) or a specific endpoint (?endpoint_id=1).
The features of the connections resource could have been merged into the other two resources; however, it would complicate the API specification and hamper the consistency and manageability upon business expansion. 

## Source code files:
The project has been developed with a focus on robustness, scalability, performance and maintainability, allowing for the easy addition of new REST resources as requirements evolve. Below is an overview of the purpose of each source file included in the project:
- database.*: This file defines a class named DataBase, which provides the basic interfaces necessary to simulate the behaviours of a NoSQL document database. The class employs an std::map to implement a temporary in-memory storage solution that is not currently persisted to the hard drive. It is capable of storing a JSON document corresponding to each individual entry of every REST resource.
- resource.*: This file contains a class named Resource, which can be configured to initiate a REST endpoint associated with a specific resource. Multiple instances of this class can be configured and executed simultaneously, enabling multiple live endpoints at any given time. The constructor of this class requires a reference to a database, a host address, and a resource name upon instantiation. While instances of this class are fully functional, they can also be extended, allowing additional features to be integrated by overriding or overloading its methods.
-	network.*, endpoint.*, connections.*: These files include classes, which extends the Resource class to introduce extra sanity checks and actions for various HTTP methods.
-	web-server.*: This file houses the WebServer class, which creates three Resource objects for each of the resources. The WebServer class also provides public methods to start or stop these endpoints concurrently.
All source files contain inline comments that offer detailed explanations of their workflow, facilitating easier navigation and understanding as you explore the code. Extensive error handlings in the request-body and the URI used have also been added. 

## Usage of programming language
I have selected C++ as the development language for this project over C and Rust. The features of C++, including its support for Object-Oriented Programming and the Standard Template Library, contribute to the creation of more robust, maintainable, and scalable applications. Although Rust is designed with a focus on safety and concurrency, C++ offers a richer ecosystem, enables a faster development process, delivers superior runtime performance, and provides greater control over system resources.

## Usage of library
While numerous C++ libraries are available for building HTTP servers that provides RESTful APIs, I have opted for the cpprestsdk (also known as Casablanca) due to several key advantages. These include its lightweight nature, asynchronous programming model, cross-platform support, robust JSON handling capabilities, comprehensive documentation, active development, and a strong community of support. Originally developed by Microsoft, the cpprestsdk is extensively utilized within their Azure ecosystem. Additionally, organizations such as Intel and Zynga have adopted this framework for their development needs.

## Endpoint Addition Strategy
Whenever a new endpoint is added to a VXLAN network or removed from the same, relevant linux commands are being written into a log file (command.log) in the same directory. This is more beneficial over the other option as these commands can be directly sent to the endpoint and run in a real-world scenario to achieve the intended effect (Addition to / removal from a VXLAN network). This dynamically generated log files containing commands can help into automation of network configuration or batch-processing. 

## Scalability aspects
The PoC has been developed keeping scalability, maintainability and performance always in mind. 
- New resources can easily be added anytime by extending the Resource class.  -
- To productize this PoC, decreasing number of database queries and string operations will be my first priority. I would also prefer to modify the schema to align them with the usual query patterns.
- Source code can also be refactored more by following design patterns (specifically Factory, Proxy, Prototype etc.), SOLID and DRY principles.
- A home-made in-memory database has been used for the purpose of the PoC. It can be persisted into a file everytime the webserver is closed and read back again when it is restarted.
- During productization, I would prefer usage of a NoSQL Document Database (such as MongoDB) that can store JSONs efficiently can horizontally distribute the data to provide redundancy, speed and fault-tolerance.   


## Performance Aspects
In this type of systems, API throughput and latencies can be the performance bottlenecks specially with respect to North & South bound APIs. This may hamper scalability of communication between controller and application when the system grows to support millions of tenant networks and endpoints. In terms of Data Plane, flow rule installations and speed of packet processing can be potential bottlenecks. To mitigate this, high-speed packet processing pipeline can be utilized by using frameworks like Snabb or DPDK. Programmable NICs (SmartNICs) or SR-IOV can also be integrated to offload packet processing tasks from system CPUs. 

## Security Considerations
The PoC doesn’t integrate SSL in the implementation. So, all the request or response bodies are vulnerable to middle-man attack due to lack of provision for Authenticity, Integrity and Privacy. The cpprestsdk library comes with necessary features of integrating OpenSSL into the project which can be incorporated during productization. 

## Control Plane Integration
Popular Control Plane Protocols such as BGP EVPN can easily be integrated with this system to facilitate seamless control-plane learning and topology discovery. Each tenant networks can act as Autonomous Systems (ASs) in terms of BGP protocol and can enable learning of VTEPs and endpoints without flooding the network.  Flow Table rules added by this system can enable efficient MAC learning when a new endpoint is added or removed.

## Assumptions & Trade-offs
Few assumptions have been made while developing the PoC to prioritize simplicity of the codebase over providing complex features and validation checks.
- A whole Tenant Network cannot be deleted once it has been added, even when all the relevant VXLAN networks has been deleted using “DELETE /networks/{tenantId}/{vni}” call. There is no provision of “DELETE /networks/{tenantId}” as of now. It can be implemented easily through adding some extra line of code in “handleDelete()” function in the “resource.cpp” file.
- Ideally when an endpoint gets deleted (DELETE /endpoints/endpoint_id) or a VNI gets deleted (DELETE /networks/{tenantId}/{vni}), associated connection in the connections resource should also be deleted. This is also not been handled. This can also be handled from the “handleDelete()” function.
- While adding a new endpoint (POST /endpoints), provided MAC address is not checked for format-validation. It can be done using the same process using which IP addresses are being validated in networks.cpp and endpoints.cpp.
