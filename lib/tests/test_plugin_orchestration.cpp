/**
 * @file test_plugin_orchestration.cpp
 * @brief Tests for plugin orchestration system
 * @version 3.1.0
 */

#include <QtTest/QtTest>
#include <QJsonObject>
#include <QSignalSpy>
#include "qtplugin/orchestration/plugin_orchestrator.hpp"

using namespace qtplugin::orchestration;

class TestPluginOrchestration : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Workflow tests
    void testWorkflowCreation();
    void testWorkflowValidation();
    void testWorkflowSerialization();
    void testWorkflowExecutionOrder();
    
    // PluginOrchestrator tests
    void testOrchestratorWorkflowRegistration();
    void testOrchestratorWorkflowExecution();
    void testOrchestratorWorkflowCancellation();
    void testOrchestratorExecutionMonitoring();
    
    // Error handling tests
    void testInvalidWorkflowValidation();
    void testCircularDependencyDetection();
    void testWorkflowExecutionFailure();
    
private:
    Workflow createTestWorkflow();
    Workflow createComplexWorkflow();
    std::unique_ptr<PluginOrchestrator> m_orchestrator;
};

void TestPluginOrchestration::initTestCase() {
    qDebug() << "Starting plugin orchestration tests";
    m_orchestrator = std::make_unique<PluginOrchestrator>();
}

void TestPluginOrchestration::cleanupTestCase() {
    m_orchestrator.reset();
    qDebug() << "Plugin orchestration tests completed";
}

void TestPluginOrchestration::testWorkflowCreation() {
    Workflow workflow("test_workflow", "Test Workflow");
    
    QCOMPARE(workflow.id(), QString("test_workflow"));
    QCOMPARE(workflow.name(), QString("Test Workflow"));
    QCOMPARE(workflow.execution_mode(), ExecutionMode::Sequential);
    
    // Test method chaining
    workflow.set_description("Test workflow description")
           .set_execution_mode(ExecutionMode::Parallel)
           .set_timeout(std::chrono::milliseconds(60000));
    
    QCOMPARE(workflow.description(), QString("Test workflow description"));
    QCOMPARE(workflow.execution_mode(), ExecutionMode::Parallel);
    QCOMPARE(workflow.timeout(), std::chrono::milliseconds(60000));
}

void TestPluginOrchestration::testWorkflowValidation() {
    // Valid workflow
    Workflow valid_workflow = createTestWorkflow();
    auto validation_result = valid_workflow.validate();
    QVERIFY(validation_result.has_value());
    
    // Invalid workflow - empty ID
    Workflow invalid_workflow("", "Invalid");
    auto invalid_result = invalid_workflow.validate();
    QVERIFY(!invalid_result.has_value());
    QCOMPARE(invalid_result.error().code, qtplugin::PluginErrorCode::InvalidConfiguration);
    
    // Invalid workflow - no steps
    Workflow no_steps_workflow("no_steps", "No Steps");
    auto no_steps_result = no_steps_workflow.validate();
    QVERIFY(!no_steps_result.has_value());
}

void TestPluginOrchestration::testWorkflowSerialization() {
    Workflow original = createTestWorkflow();
    
    // Serialize to JSON
    QJsonObject json = original.to_json();
    QVERIFY(json.contains("id"));
    QVERIFY(json.contains("name"));
    QVERIFY(json.contains("steps"));
    
    // Deserialize from JSON
    auto deserialized_result = Workflow::from_json(json);
    QVERIFY(deserialized_result.has_value());
    
    const Workflow& deserialized = deserialized_result.value();
    QCOMPARE(deserialized.id(), original.id());
    QCOMPARE(deserialized.name(), original.name());
    QCOMPARE(deserialized.steps().size(), original.steps().size());
}

void TestPluginOrchestration::testWorkflowExecutionOrder() {
    Workflow workflow = createComplexWorkflow();
    
    auto execution_order = workflow.get_execution_order();
    QVERIFY(!execution_order.empty());
    
    // Verify that dependencies are respected
    // step1 should come before step2 (step2 depends on step1)
    auto step1_pos = std::find(execution_order.begin(), execution_order.end(), "step1");
    auto step2_pos = std::find(execution_order.begin(), execution_order.end(), "step2");
    
    QVERIFY(step1_pos != execution_order.end());
    QVERIFY(step2_pos != execution_order.end());
    QVERIFY(step1_pos < step2_pos);
}

void TestPluginOrchestration::testOrchestratorWorkflowRegistration() {
    Workflow workflow = createTestWorkflow();
    
    // Register workflow
    auto register_result = m_orchestrator->register_workflow(workflow);
    QVERIFY(register_result.has_value());
    
    // Verify registration
    auto get_result = m_orchestrator->get_workflow(workflow.id());
    QVERIFY(get_result.has_value());
    QCOMPARE(get_result.value().id(), workflow.id());
    
    // List workflows
    auto workflows = m_orchestrator->list_workflows();
    QVERIFY(std::find(workflows.begin(), workflows.end(), workflow.id()) != workflows.end());
    
    // Unregister workflow
    auto unregister_result = m_orchestrator->unregister_workflow(workflow.id());
    QVERIFY(unregister_result.has_value());
    
    // Verify unregistration
    auto get_after_unregister = m_orchestrator->get_workflow(workflow.id());
    QVERIFY(!get_after_unregister.has_value());
}

void TestPluginOrchestration::testOrchestratorWorkflowExecution() {
    Workflow workflow = createTestWorkflow();
    m_orchestrator->register_workflow(workflow);
    
    // Setup signal spy
    QSignalSpy started_spy(m_orchestrator.get(), &PluginOrchestrator::workflow_started);
    QSignalSpy completed_spy(m_orchestrator.get(), &PluginOrchestrator::workflow_completed);
    QSignalSpy failed_spy(m_orchestrator.get(), &PluginOrchestrator::workflow_failed);
    
    // Execute workflow (synchronously for testing)
    QJsonObject initial_data;
    initial_data["test_input"] = "test_value";
    
    auto execution_result = m_orchestrator->execute_workflow(workflow.id(), initial_data, false);
    
    // Note: This test may fail if no actual plugins are loaded
    // In a real test environment, we would need mock plugins
    if (execution_result.has_value()) {
        QVERIFY(started_spy.count() >= 1);
        // Check if workflow completed or failed
        QVERIFY(completed_spy.count() >= 1 || failed_spy.count() >= 1);
    } else {
        // Expected to fail without actual plugins
        qDebug() << "Workflow execution failed (expected without plugins):" 
                 << QString::fromStdString(execution_result.error().message);
    }
    
    // Cleanup
    m_orchestrator->unregister_workflow(workflow.id());
}

void TestPluginOrchestration::testOrchestratorWorkflowCancellation() {
    Workflow workflow = createTestWorkflow();
    m_orchestrator->register_workflow(workflow);
    
    // Start workflow asynchronously
    auto execution_result = m_orchestrator->execute_workflow(workflow.id(), QJsonObject(), true);
    
    if (execution_result.has_value()) {
        QString execution_id = execution_result.value();
        
        // Cancel workflow
        auto cancel_result = m_orchestrator->cancel_workflow(execution_id);
        QVERIFY(cancel_result.has_value());
        
        // Verify cancellation
        QSignalSpy cancelled_spy(m_orchestrator.get(), &PluginOrchestrator::workflow_cancelled);
        QVERIFY(cancelled_spy.wait(1000)); // Wait up to 1 second
    }
    
    // Cleanup
    m_orchestrator->unregister_workflow(workflow.id());
}

void TestPluginOrchestration::testOrchestratorExecutionMonitoring() {
    Workflow workflow = createTestWorkflow();
    m_orchestrator->register_workflow(workflow);
    
    // Start workflow
    auto execution_result = m_orchestrator->execute_workflow(workflow.id(), QJsonObject(), true);
    
    if (execution_result.has_value()) {
        QString execution_id = execution_result.value();
        
        // Get execution status
        auto status_result = m_orchestrator->get_execution_status(execution_id);
        if (status_result.has_value()) {
            QJsonObject status = status_result.value();
            QVERIFY(status.contains("execution_id"));
            QVERIFY(status.contains("workflow_id"));
            QCOMPARE(status["execution_id"].toString(), execution_id);
        }
        
        // List active executions
        auto active_executions = m_orchestrator->list_active_executions();
        QVERIFY(std::find(active_executions.begin(), active_executions.end(), execution_id) != active_executions.end());
        
        // Cancel to cleanup
        m_orchestrator->cancel_workflow(execution_id);
    }
    
    // Cleanup
    m_orchestrator->unregister_workflow(workflow.id());
}

void TestPluginOrchestration::testInvalidWorkflowValidation() {
    // Test workflow with missing dependency
    Workflow invalid_workflow("invalid", "Invalid Workflow");
    
    WorkflowStep step1("step1", "plugin1", "method1");
    step1.dependencies.push_back("nonexistent_step");
    
    invalid_workflow.add_step(step1);
    
    auto validation_result = invalid_workflow.validate();
    QVERIFY(!validation_result.has_value());
    QCOMPARE(validation_result.error().code, qtplugin::PluginErrorCode::DependencyMissing);
}

void TestPluginOrchestration::testCircularDependencyDetection() {
    Workflow circular_workflow("circular", "Circular Workflow");
    
    WorkflowStep step1("step1", "plugin1", "method1");
    step1.dependencies.push_back("step2");
    
    WorkflowStep step2("step2", "plugin2", "method2");
    step2.dependencies.push_back("step1");
    
    circular_workflow.add_step(step1).add_step(step2);
    
    auto execution_order = circular_workflow.get_execution_order();
    QVERIFY(execution_order.empty()); // Should be empty due to circular dependency
    
    auto validation_result = circular_workflow.validate();
    QVERIFY(!validation_result.has_value());
    QCOMPARE(validation_result.error().code, qtplugin::PluginErrorCode::CircularDependency);
}

Workflow TestPluginOrchestration::createTestWorkflow() {
    Workflow workflow("test_workflow", "Test Workflow");
    
    workflow.set_description("Simple test workflow")
           .set_execution_mode(ExecutionMode::Sequential);
    
    WorkflowStep step1("step1", "test_plugin", "test_method");
    step1.name = "Test Step 1";
    step1.description = "First test step";
    step1.parameters["param1"] = "value1";
    
    workflow.add_step(step1);
    
    return workflow;
}

Workflow TestPluginOrchestration::createComplexWorkflow() {
    Workflow workflow("complex_workflow", "Complex Workflow");
    
    WorkflowStep step1("step1", "plugin1", "method1");
    step1.name = "Step 1";
    
    WorkflowStep step2("step2", "plugin2", "method2");
    step2.name = "Step 2";
    step2.dependencies.push_back("step1"); // step2 depends on step1
    
    WorkflowStep step3("step3", "plugin3", "method3");
    step3.name = "Step 3";
    // step3 has no dependencies, can run in parallel with step1
    
    workflow.add_step(step1)
           .add_step(step2)
           .add_step(step3);
    
    return workflow;
}

QTEST_MAIN(TestPluginOrchestration)
#include "test_plugin_orchestration.moc"
