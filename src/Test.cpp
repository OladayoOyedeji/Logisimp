#include "Test.h"

int TestProject::available_circuits()
{
    // choose project
    // TODO: a try catch block
    cout << "Project --\n";
    cout << "         |\n";
    cout << "         v\n";
    
    for (auto p: sim)
    {
        cout << "      " << p.first << endl;
    }
    cout << "EOF\n";
    return 1;
}

int TestProject::toggle_inputs(string & ProjectName)
{
    cout << "Project Name: ";
    cin >> ProjectName;
    cout << "Complete\n";
    if (sim.find(ProjectName) == sim.end())
    {
        return 1;
    }

    cout << "values for inputs\n";
    Project & p = *sim[ProjectName];
    for (int i = 0; i < p.input_size(); ++i)
    {
        LogicVector value;
        int width = p.output_width(i);

        cout << "Values\n";
        for (int j = 0; j < width; ++j)
        {
            cin >> value[j];
        }
        p.set_at(i, value);
    }

    return 2;
}

int TestProject::show_outputs(const string & ProjectName)
{
    cout << "values for outputs\n";
    Project & p = *sim[ProjectName];
    p.sim_run();
    for (int i = 0; i < p.output_size(); ++i)
    {
        // LogicVector value;
        // int width = p.input_width(i);
        // for (int j = 0; j < width; ++j)
        // {
        //     cout << value[j] << endl;
        // }
        cout << p.outputs_[i]->value()[0] << endl;
    }

    return 0;
}

void TestProject::simi_run()
{
    string ProjectName(1024, ' ');
    int i = 0;
    
    while (1)
    {
        if (i == 0)
            i = available_circuits();
        else if (i == 1)
            i = toggle_inputs(ProjectName); 
        else if (i == 2)
            i = show_outputs(ProjectName);
        else
            break;
    }
}

void TestProject::test_input_output()
{
    const char * name = "input_output";
    sim[name] = new Project;
    Project & project = *sim[name];
    Pin & a = project.add_pin(INPUT_PIN);
    Pin & b = project.add_pin(OUTPUT_PIN);

    Wire & wire = project.create_wire();
    project.connect(*a.outputs()[0], wire);
    project.connect(*b.inputs()[0], wire);

    project.initialize();
}

void TestProject::test_NOT()
{
    const char * name = "NOT";
    sim[name] = new Project;
    
    Project & project = *sim[name];
    Pin & a = project.add_pin(INPUT_PIN);
    NotGate & Not = project.create_not_gate();
    Pin & b = project.add_pin(OUTPUT_PIN);

    Wire & wireA = project.create_wire();
    project.connect(*a.outputs()[0], wireA);
    project.connect(*Not.inputs()[0], wireA);

    Wire & wireB = project.create_wire();
    project.connect(*Not.outputs()[0], wireB);
    project.connect(*b.inputs()[0], wireB);
    
    project.initialize();
}

void TestProject::test_AND()
{
    const char * name = "AND";
    sim[name] = new Project;

    Project & project = *sim[name];

    Pin & a = project.add_pin(INPUT_PIN);
    Pin & b = project.add_pin(INPUT_PIN);
    AndGate & And = project.create_and_gate();
    Pin & c = project.add_pin(OUTPUT_PIN);

    Wire & wireA = project.create_wire();
    Wire & wireB = project.create_wire();
    Wire & wireC = project.create_wire();

    project.connect(*a.outputs()[0], wireA);
    project.connect(*And.inputs()[0], wireA);

    
    project.connect(*b.outputs()[0], wireB);
    project.connect(*And.inputs()[1], wireB);

    project.connect(*c.inputs()[0], wireC);
    project.connect(*And.outputs()[0], wireC);
    
    project.initialize();
}

void TestProject::test_OR()
{
    const char * name = "NOT";
    sim[name] = new Project;

    Project & project = *sim[name];

    Pin & a = project.add_pin(INPUT_PIN);
    Pin & b = project.add_pin(INPUT_PIN);
    OrGate & Or = project.create_or_gate();
    Pin & c = project.add_pin(OUTPUT_PIN);

    Wire & wireA = project.create_wire();
    Wire & wireB = project.create_wire();
    Wire & wireC = project.create_wire();

    project.connect(*a.outputs()[0], wireA);
    project.connect(*Or.inputs()[0], wireA);

    
    project.connect(*b.outputs()[0], wireB);
    project.connect(*Or.inputs()[1], wireB);

    project.connect(*c.inputs()[0], wireC);
    project.connect(*Or.outputs()[0], wireC);
    
    project.initialize();
}

void TestProject::test_NAND()
{
    const char * name = "NOT";
    sim[name] = new Project;

    Project & project = *sim[name];

    Pin & a = project.add_pin(INPUT_PIN);
    Pin & b = project.add_pin(INPUT_PIN);
    NandGate & Nand = project.create_nand_gate();
    Pin & c = project.add_pin(OUTPUT_PIN);

    Wire & wireA = project.create_wire();
    Wire & wireB = project.create_wire();
    Wire & wireC = project.create_wire();

    project.connect(*a.outputs()[0], wireA);
    project.connect(*Nand.inputs()[0], wireA);

    
    project.connect(*b.outputs()[0], wireB);
    project.connect(*Nand.inputs()[1], wireB);

    project.connect(*c.inputs()[0], wireC);
    project.connect(*Nand.outputs()[0], wireC);
    
    project.initialize();
}


void test_half_adder()
{
    Circuit circuit;
    Simulator simulator;

    Pin & a = circuit.create_pin(INPUT_PIN);
    Pin & b = circuit.create_pin(INPUT_PIN);

    Pin & sum = circuit.create_pin(OUTPUT_PIN);
    Pin & carry = circuit.create_pin(OUTPUT_PIN);

    XorGate & xor_gate = circuit.create_xor_gate();
    AndGate & and_gate = circuit.create_and_gate();

    Wire & a_wire = circuit.create_wire();
    Wire & b_wire = circuit.create_wire();
    Wire & sum_wire = circuit.create_wire();
    Wire & carry_wire = circuit.create_wire();

    circuit.connect(*a.outputs()[0], a_wire);
    circuit.connect(*xor_gate.inputs()[0], a_wire);
    circuit.connect(*and_gate.inputs()[0], a_wire);

    circuit.connect(*b.outputs()[0], b_wire);
    circuit.connect(*xor_gate.inputs()[1], b_wire);
    circuit.connect(*and_gate.inputs()[1], b_wire);

    circuit.connect(*xor_gate.output(), sum_wire);
    circuit.connect(*sum.inputs()[0], sum_wire);

    circuit.connect(*and_gate.output(), carry_wire);
    circuit.connect(*carry.inputs()[0], carry_wire);

    circuit.initialize(simulator);

    std::cout << "A B | SUM CARRY" << std::endl;
    std::cout << "----|----------" << std::endl;

    for (int a_value = 0; a_value <= 1; a_value++)
    {
        for (int b_value = 0; b_value <= 1; b_value++)
        {
            a.set(LogicVector(1, a_value == 0 ? LOW : HIGH), simulator);
            b.set(LogicVector(1, b_value == 0 ? LOW : HIGH), simulator);

            simulator.run();

            std::cout << a_value << ' '
                      << b_value << " | "
                      << sum.value()[0] << ' '
                      << carry.value()[0] << '\n';
        }
    }
}
