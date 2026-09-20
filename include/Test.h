#ifndef TEST_H
#define TEST_H

// Simulator:
#include <unordered_map>
#include "LogicValue.h"
#include "LogicVector.h"
#include "Port.h"
#include "Component.h"
#include "Wire.h"
#include "Simulator.h"
#include "Circuit.h"
#include "Gate.h"
#include "Gates.h"
#include "Pin.h"
//#include "Clock.h"
//#include "Mux.h"
//#include "Adder.h"
//#include "Register.h"
//#include "RAM.h"
//#include "ROM.h"

class Project: public Circuit
{
public:
    Project()
        : simulator(new Simulator)
    {}
    ~Project()
    {
        delete simulator;
    }
    void print_truth_table(); // TODO
    Pin & add_pin(PinType type, int width = 1)
    {
        Pin & p = create_pin(type, width);
        if (type == INPUT_PIN)
            inputs_.push_back(&p);
        else
            outputs_.push_back(&p);

        
        return p;
    }
    int input_size() const
    {
        return inputs_.size();
    }
    
    int output_size() const
    {
        return outputs_.size();
    }

    int input_width(int i) const
    {
        // cout << i << ' ' << inputs_[i] << endl;
        // Component * c = component_at(inputs_[i]);
        // cout << "get component: " << c << endl;
        // cout << "size: " << c->input_count() << endl;
        // cout << "input port at " << c->input_port(0) << endl;
        return (outputs_[i]->input_port(0))->width();
    }
    
    int output_width(int i) const
    {
        return (inputs_[i]->output_port(0))->width();
    }

    bool initialize()
    {
        return Circuit::initialize(*simulator);
    }

    void set_at(int i, const LogicVector & value)
    {
        Pin & p = *(inputs_[i]);
        p.set(value, *simulator);
    }

    void sim_run()
    {
        simulator->run();
    }
    
    std::vector<Pin *> inputs_;
    std::vector<Pin *> outputs_; 
    Simulator * simulator;
};

class TestProject
{
public:
    TestProject()  //(const string & s) for directory
    {
        sim["main"] = new Project;
        test_input_output();
        
        test_NOT();
        test_AND();
        test_OR();
        test_NAND();
        // test_NOR();
        // test_XOR();
        // test_XNOR();

        // test_BUFFER();
        // test_TRI_STATE_BUFFER();

        // test_HALF_ADDER();
        // test_FULL_ADDER();
        // test_4BIT_ADDER();

        // test_MULTIPLEXER();
        // test_4_1_MUX();
        // test_8_1_MUX();
        // test_16_1_MUX();
        // test_DEMULTIPLEXER();
        // test_DECODER();
        // test_ENCODER();
    }
    ~TestProject()
    {
        for (auto p : sim)
        {
            delete p.second;
        }
        
    }
    void simi_run();
    int available_circuits();
    int project_options(const string & name)
    {
        Project & pr = *sim[name];
        int outputs;
        cin >> outputs;

        return outputs;
    }
    void availiable_projects();
    int toggle_inputs(string &);
    int show_outputs(const string &);
    
    void test_input_output();
    void test_NOT();
    void test_AND();
    void test_OR();
    void test_NAND();
    void test_NOR();
    void test_XOR();
    void test_XNOR();

    void test_BUFFER();
    void test_TRI_STATE_BUFFER();

    void test_HALF_ADDER();
    void test_FULL_ADDER();
    void test_4BIT_ADDER();

    void test_MULTIPLEXER();
    void test_4_1_MUX();
    void test_8_1_MUX();
    void test_16_1_MUX();
    void test_DEMULTIPLEXER();
    void test_DECODER();
    void test_ENCODER();
private:
    unordered_map<string, Project *> sim;
};
#endif
