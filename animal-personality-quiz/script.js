// This file contains the JavaScript logic for the animal personality quiz.

document.addEventListener('DOMContentLoaded', () => {
    const form = document.getElementById('quiz-form');
    const submitBtn = document.getElementById('submit-btn');
    const resultContainer = document.getElementById('result');
    
    fetch('data/questions.csv')
        .then(response => response.text())
        .then(data => {
            const questions = parseCSV(data);
            loadQuestions(questions);
        });

    submitBtn.addEventListener('click', (event) => {
        event.preventDefault();
        const formData = new FormData(form);
        let score = 0;

        // Process form data and calculate score
        formData.forEach((value, key) => {
            score += parseInt(value);
        });

        // Determine result based on score
        let result;
        if (score < 10) {
            result = 'You are a Cat!';
        } else if (score < 20) {
            result = 'You are a Dog!';
        } else {
            result = 'You are an Elephant!';
        }

        // Display result
        resultContainer.textContent = result;
    });

    function parseCSV(data) {
        const rows = data.split('\n').map(row => row.split(','));
        return rows;
    }

    function loadQuestions(questions) {
        questions.forEach((question, index) => {
            const questionElement = document.createElement('div');
            questionElement.innerHTML = `<label>${question[0]}</label>`;
            question.slice(1).forEach(option => {
                questionElement.innerHTML += `<input type="radio" name="question${index}" value="${option}">${option}<br>`;
            });
            form.appendChild(questionElement);
        });
    }

    function getAnswers() {
        const answers = [];
        const inputs = form.querySelectorAll('input[type="radio"]:checked');
        inputs.forEach(input => {
            answers.push(input.value);
        });
        return answers;
    }

    function calculateResult(answers, results) {
        // Simple scoring logic based on answers
        const score = answers.reduce((acc, answer) => acc + (answer.length % 3), 0);
        return results[score % results.length][0]; // Return animal based on score
    }

    function displayResult(result) {
        resultSection.innerHTML = `<h2>You are a ${result}!</h2>`;
    }
});